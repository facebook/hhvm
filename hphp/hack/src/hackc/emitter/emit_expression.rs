// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::collections::BTreeMap;
use std::iter;
use std::str::FromStr;

use emit_pos::emit_pos;
use emit_pos::emit_pos_then;
use env::emitter::Emitter;
use env::ClassExpr;
use env::Env;
use env::Flags as EnvFlags;
use error::Error;
use error::Result;
use ffi::Str;
use hash::HashSet;
use hhbc::AsTypeStructExceptionKind;
use hhbc::BareThisOp;
use hhbc::ClassGetCMode;
use hhbc::CollectionType;
use hhbc::FCallArgs;
use hhbc::FCallArgsFlags;
use hhbc::IncDecOp;
use hhbc::IncludePath;
use hhbc::Instruct;
use hhbc::IsLogAsDynamicCallOp;
use hhbc::IsTypeOp;
use hhbc::IterArgs;
use hhbc::Label;
use hhbc::Local;
use hhbc::MOpMode;
use hhbc::MemberKey;
use hhbc::MethodName;
use hhbc::OODeclExistsOp;
use hhbc::ObjMethodOp;
use hhbc::Opcode;
use hhbc::QueryMOp;
use hhbc::ReadonlyOp;
use hhbc::SetOpOp;
use hhbc::SetRangeOp;
use hhbc::SpecialClsRef;
use hhbc::StackIndex;
use hhbc::TypeStructResolveOp;
use hhbc::TypedValue;
use hhbc_string_utils as string_utils;
use indexmap::IndexSet;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use itertools::Itertools;
use lazy_static::lazy_static;
use naming_special_names_rust::emitter_special_functions;
use naming_special_names_rust::fb;
use naming_special_names_rust::pseudo_consts;
use naming_special_names_rust::pseudo_functions;
use naming_special_names_rust::special_functions;
use naming_special_names_rust::special_idents;
use naming_special_names_rust::superglobals;
use naming_special_names_rust::typehints;
use naming_special_names_rust::user_attributes;
use oxidized::aast;
use oxidized::aast_defs;
use oxidized::aast_visitor::visit;
use oxidized::aast_visitor::visit_mut;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::Node;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::Visitor;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::ast_defs::ParamKind;
use oxidized::local_id;
use oxidized::pos::Pos;
use regex::Regex;
use serde_json::json;
use string_utils::reified::ReifiedTparam;

use super::TypeRefinementInHint;
use crate::emit_adata;
use crate::emit_fatal;
use crate::emit_type_constant;

#[derive(Debug)]
pub struct EmitJmpResult<'arena> {
    /// Generated instruction sequence.
    pub instrs: InstrSeq<'arena>,
    /// Does instruction sequence fall through?
    is_fallthrough: bool,
    /// Was label associated with emit operation used?
    is_label_used: bool,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum LValOp {
    Set,
    SetOp(SetOpOp),
    IncDec(IncDecOp),
    Unset,
}

impl LValOp {
    fn is_incdec(&self) -> bool {
        if let Self::IncDec(_) = self {
            return true;
        };
        false
    }
}

pub fn is_local_this<'a, 'arena>(env: &Env<'a, 'arena>, lid: &local_id::LocalId) -> bool {
    local_id::get_name(lid) == special_idents::THIS
        && env.scope.has_this()
        && !env.scope.is_toplevel()
}

mod inout_locals {
    use std::marker::PhantomData;

    use hash::HashMap;
    use oxidized::aast_defs::Lid;
    use oxidized::aast_visitor;
    use oxidized::aast_visitor::Node;
    use oxidized::ast;
    use oxidized::ast_defs;

    use super::Emitter;
    use super::Env;
    use super::Local;
    use super::ParamKind;

    pub(super) struct AliasInfo {
        first_inout: isize,
        last_write: isize,
        num_uses: usize,
    }

    impl Default for AliasInfo {
        fn default() -> Self {
            AliasInfo {
                first_inout: std::isize::MAX,
                last_write: std::isize::MIN,
                num_uses: 0,
            }
        }
    }

    impl AliasInfo {
        pub(super) fn add_inout(&mut self, i: isize) {
            if i < self.first_inout {
                self.first_inout = i;
            }
        }

        pub(super) fn add_write(&mut self, i: isize) {
            if i > self.last_write {
                self.last_write = i;
            }
        }

        pub(super) fn add_use(&mut self) {
            self.num_uses += 1
        }

        pub(super) fn in_range(&self, i: isize) -> bool {
            i > self.first_inout || i <= self.last_write
        }

        pub(super) fn has_single_ref(&self) -> bool {
            self.num_uses < 2
        }
    }

    pub(super) type AliasInfoMap<'ast> = HashMap<&'ast str, AliasInfo>;

    pub(super) fn new_alias_info_map<'ast>() -> AliasInfoMap<'ast> {
        HashMap::default()
    }

    fn add_write<'ast>(name: &'ast str, i: usize, map: &mut AliasInfoMap<'ast>) {
        map.entry(name.as_ref()).or_default().add_write(i as isize);
    }

    fn add_inout<'ast>(name: &'ast str, i: usize, map: &mut AliasInfoMap<'ast>) {
        map.entry(name.as_ref()).or_default().add_inout(i as isize);
    }

    fn add_use<'ast>(name: &'ast str, map: &mut AliasInfoMap<'ast>) {
        map.entry(name.as_ref()).or_default().add_use();
    }

    // determines if value of a local 'name' that appear in parameter 'i'
    // should be saved to local because it might be overwritten later
    pub(super) fn should_save_local_value(
        name: &str,
        i: usize,
        aliases: &AliasInfoMap<'_>,
    ) -> bool {
        aliases
            .get(name)
            .map_or(false, |alias| alias.in_range(i as isize))
    }

    pub(super) fn should_move_local_value(
        e: &Emitter<'_, '_>,
        local: Local,
        aliases: &AliasInfoMap<'_>,
    ) -> bool {
        let name = e.local_name(local);
        aliases
            .get(name.unsafe_as_str())
            .map_or(true, |alias| alias.has_single_ref())
    }

    pub(super) fn collect_written_variables<'ast, 'arena>(
        env: &Env<'ast, 'arena>,
        args: &'ast [(ParamKind, ast::Expr)],
    ) -> AliasInfoMap<'ast> {
        let mut acc = HashMap::default();
        args.iter()
            .enumerate()
            .for_each(|(i, (pk, arg))| handle_arg(env, true, i, pk, arg, &mut acc));
        acc
    }

    fn handle_arg<'ast, 'arena>(
        env: &Env<'ast, 'arena>,
        is_top: bool,
        i: usize,
        pk: &ParamKind,
        arg: &'ast ast::Expr,
        acc: &mut AliasInfoMap<'ast>,
    ) {
        use ast::Expr;
        use ast::Expr_;
        let Expr(_, _, e) = arg;
        // inout $v
        if let (ParamKind::Pinout(_), Expr_::Lvar(lid)) = (pk, e) {
            let Lid(_, lid) = &**lid;
            if !super::is_local_this(env, lid) {
                add_use(&lid.1, acc);
                return if is_top {
                    add_inout(lid.1.as_str(), i, acc);
                } else {
                    add_write(lid.1.as_str(), i, acc);
                };
            }
        }
        // $v
        if let Some(Lid(_, (_, id))) = e.as_lvar() {
            return add_use(id.as_str(), acc);
        }
        // dive into argument value
        aast_visitor::visit(
            &mut Visitor(PhantomData),
            &mut Ctx { state: acc, env, i },
            arg,
        )
        .unwrap();
    }

    struct Visitor<'r, 'arena>(PhantomData<(&'arena (), &'r ())>);

    pub struct Ctx<'r, 'ast, 'arena> {
        state: &'r mut AliasInfoMap<'ast>,
        env: &'r Env<'ast, 'arena>,
        i: usize,
    }

    impl<'r, 'ast: 'r, 'arena: 'r> aast_visitor::Visitor<'ast> for Visitor<'r, 'arena> {
        type Params = aast_visitor::AstParams<Ctx<'r, 'ast, 'arena>, ()>;

        fn object(&mut self) -> &mut dyn aast_visitor::Visitor<'ast, Params = Self::Params> {
            self
        }

        fn visit_expr_(
            &mut self,
            c: &mut Ctx<'r, 'ast, 'arena>,
            p: &'ast ast::Expr_,
        ) -> Result<(), ()> {
            // f(inout $v) or f(&$v)
            if let ast::Expr_::Call(expr) = p {
                let ast::CallExpr {
                    args, unpacked_arg, ..
                } = &**expr;
                args.iter()
                    .for_each(|(pk, arg)| handle_arg(c.env, false, c.i, pk, arg, c.state));
                if let Some(arg) = unpacked_arg.as_ref() {
                    handle_arg(c.env, false, c.i, &ParamKind::Pnormal, arg, c.state)
                }
                Ok(())
            } else {
                p.recurse(c, self.object())?;
                Ok(match p {
                    // lhs op= _
                    ast::Expr_::Binop(expr) => {
                        let ast::Binop { bop, lhs: left, .. } = &**expr;
                        if let ast_defs::Bop::Eq(_) = bop {
                            collect_lvars_hs(c, left)
                        }
                    }
                    // $i++ or $i--
                    ast::Expr_::Unop(expr) => {
                        let (uop, e) = &**expr;
                        match uop {
                            ast_defs::Uop::Uincr | ast_defs::Uop::Udecr => collect_lvars_hs(c, e),
                            _ => {}
                        }
                    }
                    // $v
                    ast::Expr_::Lvar(expr) => {
                        let Lid(_, (_, id)) = &**expr;
                        add_use(id, c.state);
                    }
                    _ => {}
                })
            }
        }
    } // impl<'ast, 'a, 'arena> aast_visitor::Visitor<'ast> for Visitor<'a, 'arena>

    // collect lvars on the left hand side of '=' operator
    fn collect_lvars_hs<'r, 'ast, 'arena>(ctx: &mut Ctx<'r, 'ast, 'arena>, expr: &'ast ast::Expr) {
        let ast::Expr(_, _, e) = expr;
        match e {
            ast::Expr_::Lvar(lid) => {
                let Lid(_, lid) = &**lid;
                if !super::is_local_this(ctx.env, lid) {
                    add_use(lid.1.as_str(), ctx.state);
                    add_write(lid.1.as_str(), ctx.i, ctx.state);
                }
            }
            ast::Expr_::List(exprs) => exprs.iter().for_each(|expr| collect_lvars_hs(ctx, expr)),
            _ => {}
        }
    }
} //mod inout_locals

pub(crate) fn get_type_structure_for_hint<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    tparams: &[&str],
    targ_map: &IndexSet<&str>,
    type_refinement_in_hint: TypeRefinementInHint,
    hint: &aast::Hint,
) -> Result<InstrSeq<'arena>> {
    let targ_map: BTreeMap<&str, i64> = targ_map
        .iter()
        .enumerate()
        .map(|(i, n)| (*n, i as i64))
        .collect();
    let tv = emit_type_constant::hint_to_type_constant(
        e.alloc,
        e.options(),
        tparams,
        &targ_map,
        hint,
        type_refinement_in_hint,
    )?;
    emit_adata::typed_value_into_instr(e, tv)
}

pub struct SetRange {
    pub op: SetRangeOp,
    pub size: usize,
    pub vec: bool,
}

/// kind of value stored in local
#[derive(Debug, Clone, Copy)]
pub enum StoredValueKind {
    Local,
    Expr,
}

/// represents sequence of instructions interleaved with temp locals.
///    <(i, None) :: rest> - is emitted i :: <rest> (commonly used for final instructions in sequence)
///    <(i, Some(l, local_kind)) :: rest> is emitted as
///
///    i
///    .try {
///      setl/popl l; depending on local_kind
///      <rest>
///    } .catch {
///      unset l
///      throw
///    }
///    unsetl l
type InstrSeqWithLocals<'arena> = Vec<(InstrSeq<'arena>, Option<(Local, StoredValueKind)>)>;

/// result of emit_array_get
enum ArrayGetInstr<'arena> {
    /// regular $a[..] that does not need to spill anything
    Regular(InstrSeq<'arena>),
    /// subscript expression used as inout argument that need to spill intermediate values:
    Inout {
        /// instruction sequence with locals to load value
        load: InstrSeqWithLocals<'arena>,
        /// instruction to set value back (can use locals defined in load part)
        store: InstrSeq<'arena>,
    },
}

struct ArrayGetBaseData<'arena, T> {
    base_instrs: T,
    cls_instrs: InstrSeq<'arena>,
    setup_instrs: InstrSeq<'arena>,
    base_stack_size: StackIndex,
    cls_stack_size: StackIndex,
}

/// result of emit_base
enum ArrayGetBase<'arena> {
    /// regular <base> part in <base>[..] that does not need to spill anything
    Regular(ArrayGetBaseData<'arena, InstrSeq<'arena>>),
    /// base of subscript expression used as inout argument that need to spill
    /// intermediate values
    Inout {
        /// instructions to load base part
        load: ArrayGetBaseData<'arena, InstrSeqWithLocals<'arena>>,
        /// instruction to load base part for setting inout argument back
        store: InstrSeq<'arena>,
    },
}

pub fn emit_nameof<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    class_id: &ast::ClassId,
) -> Result<InstrSeq<'arena>> {
    let cexpr = ClassExpr::class_id_to_class_expr(emitter, &env.scope, false, true, class_id);
    match cexpr {
        ClassExpr::Id(ast_defs::Id(_, cname)) => {
            let classid = hhbc::ClassName::from_ast_name_and_mangle(emitter.alloc, cname);
            Ok(instr::string_lit(classid.as_ffi_str()))
        }
        ClassExpr::Special(_) => Ok(InstrSeq::gather(vec![
            emit_load_class_ref(emitter, env, &class_id.1, cexpr)?,
            instr::class_name(),
        ])),
        ClassExpr::Reified(ast_defs::Id(pos, name)) => {
            get_reified_var_cexpr(emitter, env, &pos, &name)
        }
        ClassExpr::Expr(_) => Err(Error::unrecoverable(
            "emit_nameof: should be eliminated by elab",
        )),
    }
}

pub fn emit_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expression: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    stack_limit::maybe_grow(|| {
        use ast::Expr_;
        let ast::Expr(_, pos, expr) = expression;
        match expr {
            Expr_::Float(_)
            | Expr_::String(_)
            | Expr_::Int(_)
            | Expr_::Null
            | Expr_::False
            | Expr_::True => emit_lit(emitter, env, pos, expression),
            Expr_::EnumClassLabel(label) => emit_label(emitter, env, pos, label),
            Expr_::PrefixedString(e) => emit_expr(emitter, env, &e.1),
            Expr_::Nameof(target) => emit_nameof(emitter, env, target),
            Expr_::Lvar(e) => emit_lvar(emitter, env, pos, e),
            Expr_::ClassConst(e) => emit_class_const(emitter, env, pos, &e.0, &e.1),
            Expr_::Unop(e) => emit_unop(emitter, env, pos, e),
            Expr_::Binop(_) => emit_binop(emitter, env, pos, expression),
            Expr_::Pipe(e) => emit_pipe(emitter, env, e),
            Expr_::Is(is_expr) => emit_is_expr(emitter, env, pos, is_expr),
            Expr_::As(box e) => emit_as(emitter, env, pos, e),
            Expr_::Upcast(e) => emit_expr(emitter, env, &e.0),
            Expr_::Cast(e) => emit_cast(emitter, env, pos, &(e.0).1, &e.1),
            Expr_::Eif(e) => emit_conditional_expr(emitter, env, pos, &e.0, e.1.as_ref(), &e.2),
            Expr_::ArrayGet(e) => emit_array_get_expr(emitter, env, pos, e),
            Expr_::ObjGet(e) => emit_obj_get_expr(emitter, env, pos, e),
            Expr_::Call(c) => emit_call_expr(emitter, env, pos, None, false, c),
            Expr_::New(e) => emit_new(emitter, env, pos, e, false),
            Expr_::FunctionPointer(fp) => emit_function_pointer(emitter, env, pos, &fp.0, &fp.1),
            Expr_::Collection(e) => emit_named_collection_str(emitter, env, expression, e),
            Expr_::ValCollection(e) => emit_val_collection(emitter, env, pos, e, expression),
            Expr_::Pair(e) => emit_pair(emitter, env, pos, e, expression),
            Expr_::KeyValCollection(e) => {
                emit_keyval_collection_expr(emitter, env, pos, e, expression)
            }
            Expr_::Clone(e) => Ok(emit_pos_then(pos, emit_clone(emitter, env, e)?)),
            Expr_::Shape(e) => Ok(emit_pos_then(pos, emit_shape(emitter, env, expression, e)?)),
            Expr_::Await(e) => emit_await(emitter, env, pos, e),
            Expr_::ReadonlyExpr(e) => emit_readonly_expr(emitter, env, pos, e),
            Expr_::Yield(e) => emit_yield(emitter, env, pos, e),
            Expr_::Efun(e) => Ok(emit_pos_then(
                pos,
                emit_lambda(emitter, env, &e.use_, &e.closure_class_name)?,
            )),
            Expr_::ClassGet(e) => emit_class_get_expr(emitter, env, pos, e),

            Expr_::String2(es) => emit_string2(emitter, env, pos, es),
            Expr_::Id(e) => Ok(emit_pos_then(pos, emit_id(emitter, env, e)?)),
            Expr_::Xml(_) => Err(Error::unrecoverable(
                "emit_xhp: syntax should have been converted during rewriting",
            )),
            Expr_::Import(e) => emit_import(emitter, env, pos, &e.0, &e.1),
            Expr_::Omitted => Err(Error::unrecoverable(
                "emit_expr: Omitted should never be encountered by codegen",
            )),
            Expr_::Lfun(_) => Err(Error::unrecoverable(
                "expected Lfun to be converted to Efun during closure conversion emit_expr",
            )),
            Expr_::List(_) => Err(Error::fatal_parse(
                pos,
                "list() can only be used as an lvar. Did you mean to use tuple()?",
            )),
            Expr_::Tuple(e) => Ok(emit_pos_then(
                pos,
                emit_collection(emitter, env, expression, &mk_afvalues(e), None)?,
            )),

            Expr_::This | Expr_::Lplaceholder(_) | Expr_::Dollardollar(_) => {
                unimplemented!("TODO(hrust) Codegen after naming pass on AAST")
            }
            Expr_::ExpressionTree(et) => emit_expr(emitter, env, &et.runtime_expr),
            Expr_::ETSplice(_) => Err(Error::unrecoverable(
                "expression trees: splice should be erased during rewriting",
            )),
            Expr_::Invalid(_) => Err(Error::unrecoverable(
                "emit_expr: Invalid should never be encountered by codegen",
            )),
            Expr_::MethodCaller(_) | Expr_::Hole(_) => {
                unimplemented!("TODO(hrust)")
            }
            Expr_::Package(_) => Err(Error::unrecoverable(
                "package should have been converted into package_exists during rewriting",
            )),
        }
    })
}

fn emit_exprs_and_error_on_inout<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    exprs: &[(ParamKind, ast::Expr)],
    fn_name: &str,
) -> Result<InstrSeq<'arena>> {
    if exprs.is_empty() {
        Ok(instr::empty())
    } else {
        Ok(InstrSeq::gather(
            exprs
                .iter()
                .map(|(pk, expr)| match pk {
                    ParamKind::Pnormal => emit_expr(e, env, expr),
                    ParamKind::Pinout(p) => Err(Error::fatal_parse(
                        &Pos::merge(p, expr.pos()).map_err(Error::unrecoverable)?,
                        format!(
                            "Unexpected `inout` argument on pseudofunction: `{}`",
                            fn_name
                        ),
                    )),
                })
                .collect::<Result<Vec<_>>>()?,
        ))
    }
}

fn emit_exprs<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    exprs: &[ast::Expr],
) -> Result<InstrSeq<'arena>> {
    if exprs.is_empty() {
        Ok(instr::empty())
    } else {
        Ok(InstrSeq::gather(
            exprs
                .iter()
                .map(|expr| emit_expr(e, env, expr))
                .collect::<Result<Vec<_>>>()?,
        ))
    }
}

fn emit_id<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    id: &ast::Sid,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena; // Should this be emitter.alloc?
    let ast_defs::Id(p, s) = id;
    match s.as_str() {
        pseudo_consts::G__FILE__ => Ok(instr::instr(Instruct::Opcode(Opcode::File))),
        pseudo_consts::G__DIR__ => Ok(instr::instr(Instruct::Opcode(Opcode::Dir))),
        pseudo_consts::G__METHOD__ => Ok(instr::instr(Instruct::Opcode(Opcode::Method))),
        pseudo_consts::G__FUNCTION_CREDENTIAL__ => {
            Ok(instr::instr(Instruct::Opcode(Opcode::FuncCred)))
        }
        pseudo_consts::G__CLASS__ => Ok(InstrSeq::gather(vec![
            instr::self_cls(),
            instr::class_name(),
        ])),
        pseudo_consts::G__COMPILER_FRONTEND__ => Ok(instr::string(alloc, "hackc")),
        pseudo_consts::G__LINE__ => Ok(instr::int(p.info_pos_extended().1.try_into().map_err(
            |_| Error::fatal_parse(p, "error converting end of line from usize to isize"),
        )?)),
        pseudo_consts::G__NAMESPACE__ => Ok(instr::string(
            alloc,
            env.namespace.name.as_ref().map_or("", |s| &s[..]),
        )),
        pseudo_consts::EXIT => emit_exit(emitter, env, None),
        _ => {
            // panic!("TODO: uncomment after D19350786 lands")
            // let cid: ConstId = hhbc::ConstName::from_ast_name(&s);
            let cid = hhbc::ConstName::new(Str::new_str(alloc, string_utils::strip_global_ns(s)));
            emitter.add_constant_ref(cid.clone());
            Ok(emit_pos_then(
                p,
                instr::instr(Instruct::Opcode(Opcode::CnsE(cid))),
            ))
        }
    }
}

fn emit_exit<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr_opt: Option<&ast::Expr>,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![
        expr_opt.map_or_else(|| Ok(instr::int(0)), |e| emit_expr(emitter, env, e))?,
        instr::exit(),
    ]))
}

fn emit_yield<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    af: &ast::Afield,
) -> Result<InstrSeq<'arena>> {
    Ok(match af {
        ast::Afield::AFvalue(v) => {
            InstrSeq::gather(vec![emit_expr(e, env, v)?, emit_pos(pos), instr::yield_()])
        }
        ast::Afield::AFkvalue(k, v) => InstrSeq::gather(vec![
            emit_expr(e, env, k)?,
            emit_expr(e, env, v)?,
            emit_pos(pos),
            instr::yield_k(),
        ]),
    })
}

fn parse_include<'arena>(alloc: &'arena bumpalo::Bump, e: &ast::Expr) -> IncludePath<'arena> {
    fn strip_backslash(s: &mut String) {
        if s.starts_with('/') {
            *s = s[1..].into()
        }
    }
    fn split_var_lit(e: &ast::Expr) -> (String, String) {
        match &e.2 {
            ast::Expr_::Binop(x) if x.bop.is_dot() => {
                let (v, l) = split_var_lit(&x.rhs);
                if v.is_empty() {
                    let (var, lit) = split_var_lit(&x.lhs);
                    (var, format!("{}{}", lit, l))
                } else {
                    (v, String::new())
                }
            }
            ast::Expr_::String(lit) => (String::new(), lit.to_string()),
            _ => (text_of_expr(e), String::new()),
        }
    }
    let (mut var, mut lit) = split_var_lit(e);
    if var == pseudo_consts::G__DIR__ {
        var = String::new();
        strip_backslash(&mut lit);
    }
    if var.is_empty() {
        if std::path::Path::new(lit.as_str()).is_relative() {
            IncludePath::SearchPathRelative(Str::new_str(alloc, &lit))
        } else {
            IncludePath::Absolute(Str::new_str(alloc, &lit))
        }
    } else {
        strip_backslash(&mut lit);
        IncludePath::IncludeRootRelative(Str::new_str(alloc, &var), Str::new_str(alloc, &lit))
    }
}

fn text_of_expr(e: &ast::Expr) -> String {
    match &e.2 {
        ast::Expr_::String(s) => format!("\'{}\'", s),
        ast::Expr_::Id(id) => id.1.to_string(),
        ast::Expr_::Lvar(lid) => local_id::get_name(&lid.1).to_string(),
        ast::Expr_::ArrayGet(x) => match ((x.0).2.as_lvar(), x.1.as_ref()) {
            (Some(ast::Lid(_, id)), Some(e_)) => {
                format!("{}[{}]", local_id::get_name(id), text_of_expr(e_))
            }
            _ => "unknown".into(),
        },
        _ => "unknown".into(),
    }
}

fn text_of_class_id(cid: &ast::ClassId) -> String {
    match &cid.2 {
        ast::ClassId_::CIparent => "parent".into(),
        ast::ClassId_::CIself => "self".into(),
        ast::ClassId_::CIstatic => "static".into(),
        ast::ClassId_::CIexpr(e) => text_of_expr(e),
        ast::ClassId_::CI(ast_defs::Id(_, id)) => id.into(),
    }
}

fn text_of_prop(prop: &ast::ClassGetExpr) -> String {
    match prop {
        ast::ClassGetExpr::CGstring((_, s)) => s.into(),
        ast::ClassGetExpr::CGexpr(e) => text_of_expr(e),
    }
}

fn emit_import<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    flavor: &ast::ImportFlavor,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    use ast::ImportFlavor;
    let alloc = env.arena; // Should this be emitter.alloc?
    let inc = parse_include(alloc, expr);
    let filepath = e.filepath.clone();
    let resolved_inc = inc.resolve_include_roots(alloc, &e.options().hhvm.include_roots, &filepath);
    let (expr_instrs, import_op_instr) = match flavor {
        ImportFlavor::Include => (emit_expr(e, env, expr)?, instr::incl()),
        ImportFlavor::Require => (emit_expr(e, env, expr)?, instr::req()),
        ImportFlavor::IncludeOnce => (emit_expr(e, env, expr)?, instr::incl_once()),
        ImportFlavor::RequireOnce => match &resolved_inc {
            IncludePath::DocRootRelative(path) => {
                let expr = ast::Expr(
                    (),
                    pos.clone(),
                    ast::Expr_::String(path.unsafe_as_str().into()),
                );
                (emit_expr(e, env, &expr)?, instr::req_doc())
            }
            _ => (emit_expr(e, env, expr)?, instr::req_once()),
        },
    };
    e.add_include_ref(resolved_inc);
    Ok(InstrSeq::gather(vec![
        expr_instrs,
        emit_pos(pos),
        import_op_instr,
    ]))
}

fn emit_string2<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    es: &[ast::Expr],
) -> Result<InstrSeq<'arena>> {
    if es.is_empty() {
        Err(Error::unrecoverable(
            "String2 with zero araguments is impossible",
        ))
    } else if es.len() == 1 {
        Ok(InstrSeq::gather(vec![
            emit_expr(e, env, &es[0])?,
            emit_pos(pos),
            instr::cast_string(),
        ]))
    } else {
        Ok(InstrSeq::gather(vec![
            emit_two_exprs(e, env, &es[0].1, &es[0], &es[1])?,
            emit_pos(pos),
            instr::concat(),
            InstrSeq::gather(
                es[2..]
                    .iter()
                    .map(|expr| {
                        Ok(InstrSeq::gather(vec![
                            emit_expr(e, env, expr)?,
                            emit_pos(pos),
                            instr::concat(),
                        ]))
                    })
                    .collect::<Result<_>>()?,
            ),
        ]))
    }
}

fn emit_clone<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        instr::clone(),
    ]))
}

fn emit_lambda<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    ids: &[ast::CaptureLid],
    closure_class_name: &Option<String>,
) -> Result<InstrSeq<'arena>> {
    let closure_class_name = if let Some(n) = closure_class_name {
        n
    } else {
        return Err(Error::unrecoverable(
            "Closure conversion should have set closure_class_name",
        ));
    };

    let explicit_use = e
        .global_state()
        .explicit_use_set
        .contains(closure_class_name);
    let is_in_lambda = env.scope.is_in_lambda();

    Ok(InstrSeq::gather(vec![
        InstrSeq::gather(
            ids.iter()
                .map(|ast::CaptureLid(_, ast::Lid(pos, id))| {
                    match string_utils::reified::get_captured_generic(local_id::get_name(id)) {
                        Some(i) => {
                            if is_in_lambda {
                                let name = string_utils::reified::reified_generic_captured_name(i);
                                Ok(instr::c_get_l(e.named_local(name.as_str().into())))
                            } else {
                                Ok(emit_reified_generic_instrs(e, &Pos::NONE, i))
                            }
                        }
                        None => Ok({
                            let lid = get_local(e, env, pos, local_id::get_name(id))?;
                            if explicit_use {
                                instr::c_get_l(lid)
                            } else {
                                instr::cu_get_l(lid)
                            }
                        }),
                    }
                })
                .collect::<Result<Vec<_>>>()?,
        ),
        instr::create_cl(
            ids.len() as u32,
            hhbc::ClassName::from_raw_string(e.alloc, closure_class_name),
        ),
    ]))
}

pub fn emit_await<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let ast::Expr(_, _, e) = expr;
    let can_inline_gen_functions = !emitter
        .options()
        .function_is_renamable(emitter_special_functions::GENA);
    match e.as_call() {
        Some(ast::CallExpr {
            func: ast::Expr(_, _, ast::Expr_::Id(id)),
            args,
            unpacked_arg: None,
            ..
        }) if (can_inline_gen_functions
            && args.len() == 1
            && string_utils::strip_global_ns(&id.1) == emitter_special_functions::GENA) =>
        {
            inline_gena_call(emitter, env, error::expect_normal_paramkind(&args[0])?)
        }
        _ => {
            let after_await = emitter.label_gen_mut().next_regular();
            let instrs = match e {
                ast::Expr_::Call(c) => {
                    emit_call_expr(emitter, env, pos, Some(after_await.clone()), false, c)?
                }
                _ => emit_expr(emitter, env, expr)?,
            };
            Ok(InstrSeq::gather(vec![
                instrs,
                emit_pos(pos),
                instr::dup(),
                instr::is_type_c(IsTypeOp::Null),
                instr::jmp_nz(after_await.clone()),
                instr::await_(),
                instr::label(after_await),
            ]))
        }
    }
}

fn inline_gena_call<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    arg: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let load_arr = emit_expr(emitter, env, arg)?;
    let async_eager_label = emitter.label_gen_mut().next_regular();

    scope::with_unnamed_local(emitter, |e, arr_local| {
        let alloc = e.alloc;
        let before = InstrSeq::gather(vec![load_arr, instr::cast_dict(), instr::pop_l(arr_local)]);

        let inner = InstrSeq::gather(vec![
            instr::null_uninit(),
            instr::null_uninit(),
            instr::c_get_l(arr_local),
            instr::f_call_cls_method_d(
                FCallArgs::new(
                    FCallArgsFlags::default(),
                    1,
                    1,
                    vec![],
                    vec![],
                    Some(async_eager_label),
                    None,
                ),
                hhbc::MethodName::from_raw_string(alloc, "fromDict"),
                hhbc::ClassName::from_raw_string(alloc, "HH\\AwaitAllWaitHandle"),
            ),
            instr::await_(),
            instr::label(async_eager_label),
            instr::pop_c(),
            emit_iter(e, instr::c_get_l(arr_local), |val_local, key_local| {
                InstrSeq::gather(vec![
                    instr::c_get_l(val_local),
                    instr::wh_result(),
                    instr::base_l(
                        arr_local,
                        MOpMode::Define,
                        ReadonlyOp::Any, // TODO, handle await assignment statements correctly
                    ),
                    instr::set_m(0, MemberKey::EL(key_local, ReadonlyOp::Any)),
                    instr::pop_c(),
                ])
            })?,
        ]);

        let after = instr::push_l(arr_local);

        Ok((before, inner, after))
    })
}

fn emit_iter<'arena, 'decl, F: FnOnce(Local, Local) -> InstrSeq<'arena>>(
    e: &mut Emitter<'arena, 'decl>,
    collection: InstrSeq<'arena>,
    f: F,
) -> Result<InstrSeq<'arena>> {
    scope::with_unnamed_locals_and_iterators(e, |e| {
        let iter_id = e.iterator_mut().get();
        let val_id = e.local_gen_mut().get_unnamed();
        let key_id = e.local_gen_mut().get_unnamed();
        let loop_end = e.label_gen_mut().next_regular();
        let loop_next = e.label_gen_mut().next_regular();
        let iter_args = IterArgs {
            iter_id,
            key_id,
            val_id,
        };
        let iter_init = InstrSeq::gather(vec![
            collection,
            instr::iter_init(iter_args.clone(), loop_end),
        ]);
        let iterate = InstrSeq::gather(vec![
            instr::label(loop_next),
            f(val_id, key_id),
            instr::iter_next(iter_args, loop_next),
        ]);
        let iter_done = InstrSeq::gather(vec![
            instr::unset_l(val_id),
            instr::unset_l(key_id),
            instr::label(loop_end),
        ]);
        Ok((iter_init, iterate, iter_done))
    })
}

fn emit_shape<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    fl: &[(ast_defs::ShapeFieldName, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    fn extract_shape_field_name_pstring<'a, 'arena>(
        env: &Env<'a, 'arena>,
        pos: &Pos,
        field: &ast_defs::ShapeFieldName,
    ) -> Result<ast::Expr_> {
        use ast_defs::ShapeFieldName as SF;
        Ok(match field {
            SF::SFlitInt(s) => ast::Expr_::mk_int(s.1.clone()),
            SF::SFlitStr(s) => ast::Expr_::mk_string(s.1.clone()),
            SF::SFclassConst(id, p) => {
                if ClassExpr::is_reified_tparam(&env.scope, &id.1) {
                    return Err(Error::fatal_parse(
                        &id.0,
                        "Reified generics cannot be used in shape keys",
                    ));
                } else {
                    ast::Expr_::mk_class_const(
                        ast::ClassId((), pos.clone(), ast::ClassId_::CI(id.clone())),
                        p.clone(),
                    )
                }
            }
        })
    }
    let pos = &expr.1;
    // TODO(hrust): avoid clone
    let fl = fl
        .iter()
        .map(|(f, e)| {
            Ok(aast::Field(
                ast::Expr(
                    (),
                    pos.clone(),
                    extract_shape_field_name_pstring(env, pos, f)?,
                ),
                e.clone(),
            ))
        })
        .collect::<Result<Vec<_>>>()?;
    emit_expr(
        emitter,
        env,
        &ast::Expr(
            (),
            pos.clone(),
            ast::Expr_::KeyValCollection(Box::new(((pos.clone(), ast::KvcKind::Dict), None, fl))),
        ),
    )
}

fn emit_vec_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
) -> Result<InstrSeq<'arena>> {
    match constant_folder::vec_to_typed_value(e, &env.scope, fields) {
        Ok(tv) => {
            let instr = emit_adata::typed_value_into_instr(e, tv)?;
            emit_static_collection(env, None, pos, instr)
        }
        Err(_) => {
            emit_value_only_collection(e, env, pos, fields, |v| Instruct::Opcode(Opcode::NewVec(v)))
        }
    }
}

fn emit_named_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
    fields: &[ast::Afield],
    collection_type: CollectionType,
) -> Result<InstrSeq<'arena>> {
    let emit_vector_like = |e: &mut Emitter<'arena, 'decl>, collection_type| {
        Ok(if fields.is_empty() {
            emit_pos_then(pos, instr::new_col(collection_type))
        } else {
            InstrSeq::gather(vec![
                emit_vec_collection(e, env, pos, fields)?,
                instr::col_from_array(collection_type),
            ])
        })
    };
    let emit_map_or_set = |e: &mut Emitter<'arena, 'decl>, collection_type| {
        if fields.is_empty() {
            Ok(emit_pos_then(pos, instr::new_col(collection_type)))
        } else {
            emit_collection(e, env, expr, fields, Some(collection_type))
        }
    };
    use CollectionType as C;
    match collection_type {
        C::Vector | C::ImmVector => emit_vector_like(e, collection_type),
        C::Map | C::ImmMap | C::Set | C::ImmSet => emit_map_or_set(e, collection_type),
        C::Pair => Ok(InstrSeq::gather(vec![
            InstrSeq::gather(
                fields
                    .iter()
                    .map(|f| match f {
                        ast::Afield::AFvalue(v) => emit_expr(e, env, v),
                        _ => Err(Error::unrecoverable("impossible Pair argument")),
                    })
                    .collect::<Result<_>>()?,
            ),
            instr::new_pair(),
        ])),
        _ => Err(Error::unrecoverable("Unexpected named collection type")),
    }
}

fn emit_named_collection_str<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    (ast_defs::Id(pos, name), _, fields): &(
        ast::Sid,
        Option<ast::CollectionTarg>,
        Vec<ast::Afield>,
    ),
) -> Result<InstrSeq<'arena>> {
    let name = string_utils::strip_ns(name);
    let name = string_utils::types::fix_casing(name);
    let ctype = match name {
        "Vector" => CollectionType::Vector,
        "ImmVector" => CollectionType::ImmVector,
        "Map" => CollectionType::Map,
        "ImmMap" => CollectionType::ImmMap,
        "Set" => CollectionType::Set,
        "ImmSet" => CollectionType::ImmSet,
        "Pair" => CollectionType::Pair,
        _ => {
            return Err(Error::unrecoverable(format!(
                "collection: {} does not exist",
                name
            )));
        }
    };
    emit_named_collection(e, env, pos, expr, fields, ctype)
}

fn mk_afkvalues(es: impl Iterator<Item = (ast::Expr, ast::Expr)>) -> Vec<ast::Afield> {
    es.map(|(e1, e2)| ast::Afield::mk_afkvalue(e1, e2))
        .collect()
}

fn mk_afvalues(es: &[ast::Expr]) -> Vec<ast::Afield> {
    es.iter().cloned().map(ast::Afield::mk_afvalue).collect()
}

fn emit_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    fields: &[ast::Afield],
    transform_to_collection: Option<CollectionType>,
) -> Result<InstrSeq<'arena>> {
    let pos = &expr.1;
    match constant_folder::expr_to_typed_value_(e, &env.scope, expr, true /*allow_map*/) {
        Ok(tv) => {
            let instr = emit_adata::typed_value_into_instr(e, tv)?;
            emit_static_collection(env, transform_to_collection, pos, instr)
        }
        Err(_) => emit_dynamic_collection(e, env, expr, fields),
    }
}

fn emit_static_collection<'a, 'arena>(
    _env: &Env<'a, 'arena>,
    transform_to_collection: Option<CollectionType>,
    pos: &Pos,
    instr: InstrSeq<'arena>,
) -> Result<InstrSeq<'arena>> {
    let transform_instr = match transform_to_collection {
        Some(collection_type) => instr::col_from_array(collection_type),
        _ => instr::empty(),
    };
    Ok(InstrSeq::gather(vec![
        emit_pos(pos),
        instr,
        transform_instr,
    ]))
}

fn expr_and_new<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    instr_to_add_new: InstrSeq<'arena>,
    instr_to_add: InstrSeq<'arena>,
    field: &ast::Afield,
) -> Result<InstrSeq<'arena>> {
    match field {
        ast::Afield::AFvalue(v) => Ok(InstrSeq::gather(vec![
            emit_expr(e, env, v)?,
            emit_pos(pos),
            instr_to_add_new,
        ])),
        ast::Afield::AFkvalue(k, v) => Ok(InstrSeq::gather(vec![
            emit_two_exprs(e, env, &k.1, k, v)?,
            instr_to_add,
        ])),
    }
}

fn emit_container<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
    constructor: Instruct<'arena>,
    add_elem_instr: InstrSeq<'arena>,
    transform_instr: InstrSeq<'arena>,
    emitted_pos: InstrSeq<'arena>,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![
        InstrSeq::<'arena>::clone(&emitted_pos),
        instr::instr(constructor),
        fields
            .iter()
            .map(|f| {
                expr_and_new(
                    e,
                    env,
                    pos,
                    InstrSeq::<'arena>::clone(&add_elem_instr),
                    instr::add_elem_c(),
                    f,
                )
            })
            .collect::<Result<_>>()
            .map(InstrSeq::gather)?,
        emitted_pos,
        transform_instr,
    ]))
}

fn emit_keyvalue_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
    ctype: CollectionType,
    constructor: Instruct<'arena>,
) -> Result<InstrSeq<'arena>> {
    let transform_instr = instr::col_from_array(ctype);
    let add_elem_instr = InstrSeq::gather(vec![instr::dup(), instr::add_elem_c()]);
    let emitted_pos = emit_pos(pos);
    emit_container(
        e,
        env,
        pos,
        fields,
        constructor,
        add_elem_instr,
        transform_instr,
        emitted_pos,
    )
}

fn emit_array<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
    constructor: Instruct<'arena>,
) -> Result<InstrSeq<'arena>> {
    let add_elem_instr = instr::add_new_elem_c();
    let emitted_pos = emit_pos(pos);
    emit_container(
        e,
        env,
        pos,
        fields,
        constructor,
        add_elem_instr,
        instr::empty(),
        emitted_pos,
    )
}

fn non_numeric(s: &[u8]) -> bool {
    // Note(hrust): OCaml Int64.of_string and float_of_string ignore underscores
    let s = match std::str::from_utf8(s) {
        Ok(s) => s.replace('_', ""),
        Err(_) => return true, // numeric strings would be valid utf8
    };
    lazy_static! {
        static ref HEX: Regex = Regex::new(r"(?P<sign>^-?)0[xX](?P<digits>.*)").unwrap();
        static ref OCTAL: Regex = Regex::new(r"(?P<sign>^-?)0[oO](?P<digits>.*)").unwrap();
        static ref BINARY: Regex = Regex::new(r"(?P<sign>^-?)0[bB](?P<digits>.*)").unwrap();
        static ref FLOAT: Regex =
            Regex::new(r"(?P<int>\d*)\.(?P<dec>[0-9--0]*)(?P<zeros>0*)").unwrap();
        static ref NEG_FLOAT: Regex =
            Regex::new(r"(?P<int>-\d*)\.(?P<dec>[0-9--0]*)(?P<zeros>0*)").unwrap();
        static ref HEX_RADIX: u32 = 16;
        static ref OCTAL_RADIX: u32 = 8;
        static ref BINARY_RADIX: u32 = 2;
    }
    fn int_from_str(s: &str) -> Result<i64, ()> {
        // Note(hrust): OCaml Int64.of_string reads decimal, hexadecimal, octal, and binary
        (if HEX.is_match(s) {
            u64::from_str_radix(&HEX.replace(s, "${sign}${digits}"), *HEX_RADIX).map(|x| x as i64)
        } else if OCTAL.is_match(s) {
            u64::from_str_radix(&OCTAL.replace(s, "${sign}${digits}"), *OCTAL_RADIX)
                .map(|x| x as i64)
        } else if BINARY.is_match(s) {
            u64::from_str_radix(&BINARY.replace(s, "${sign}${digits}"), *BINARY_RADIX)
                .map(|x| x as i64)
        } else {
            i64::from_str(s)
        })
        .map_err(|_| ())
    }
    fn float_from_str_radix(s: &str, radix: u32) -> Result<f64, ()> {
        let i = i64::from_str_radix(&s.replace('.', ""), radix).map_err(|_| ())?;
        Ok(match s.matches('.').count() {
            0 => i as f64,
            1 => {
                let pow = s.split('.').last().unwrap().len();
                (i as f64) / f64::from(radix).powi(pow as i32)
            }
            _ => return Err(()),
        })
    }
    fn out_of_bounds(s: &str) -> bool {
        // compare strings instead of floats to avoid rounding imprecision
        if FLOAT.is_match(s) {
            FLOAT.replace(s, "${int}.${dec}").trim_end_matches('.') > i64::MAX.to_string().as_str()
        } else if NEG_FLOAT.is_match(s) {
            NEG_FLOAT.replace(s, "${int}.${dec}").trim_end_matches('.')
                > i64::MIN.to_string().as_str()
        } else {
            false
        }
    }
    fn float_from_str(s: &str) -> Result<f64, ()> {
        // Note(hrust): OCaml float_of_string ignores leading whitespace,
        // reads decimal and hexadecimal
        let s = s.trim_start();
        if HEX.is_match(s) {
            float_from_str_radix(&HEX.replace(s, "${sign}${digits}"), *HEX_RADIX)
        } else {
            let out_of_bounds =
                |f: f64| out_of_bounds(s) && (f > i64::MAX as f64 || f < i64::MIN as f64);
            let validate_float = |f: f64| {
                if out_of_bounds(f) || f.is_infinite() || f.is_nan() {
                    Err(())
                } else {
                    Ok(f)
                }
            };
            f64::from_str(s).map_err(|_| ()).and_then(validate_float)
        }
    }
    int_from_str(&s).is_err() && float_from_str(&s).is_err()
}

fn is_struct_init<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'_, 'arena>,
    fields: &[ast::Afield],
    allow_numerics: bool,
) -> Result<bool> {
    let mut are_all_keys_non_numeric_strings = true;
    let mut uniq_keys = HashSet::<bstr::BString>::default();
    for f in fields.iter() {
        if let ast::Afield::AFkvalue(key, _) = f {
            // TODO(hrust): if key is String, don't clone and call fold_expr
            let mut key = key.clone();
            constant_folder::fold_expr(&mut key, &env.scope, e)
                .map_err(|e| Error::unrecoverable(format!("{}", e)))?;
            if let ast::Expr(_, _, ast::Expr_::String(s)) = key {
                are_all_keys_non_numeric_strings =
                    are_all_keys_non_numeric_strings && non_numeric(s.as_slice());
                uniq_keys.insert(s);
            } else {
                are_all_keys_non_numeric_strings = false;
            }
            continue;
        }
        are_all_keys_non_numeric_strings = false;
    }
    let num_keys = fields.len();
    let limit = e.options().max_array_elem_size_on_the_stack;
    Ok((allow_numerics || are_all_keys_non_numeric_strings)
        && uniq_keys.len() == num_keys
        && num_keys <= limit
        && num_keys != 0)
}

fn emit_struct_array<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
) -> Result<InstrSeq<'arena>> {
    use ast::Expr;
    use ast::Expr_;
    let alloc = env.arena;
    let (keys, value_instrs): (Vec<&'arena [u8]>, _) = fields
        .iter()
        .map(|f| match f {
            ast::Afield::AFkvalue(k, v) => match k {
                Expr(_, _, Expr_::String(s)) => {
                    Ok((alloc.alloc_slice_copy(s) as &[u8], emit_expr(e, env, v)?))
                }
                _ => {
                    let mut k = k.clone();
                    constant_folder::fold_expr(&mut k, &env.scope, e)
                        .map_err(|e| Error::unrecoverable(format!("{}", e)))?;
                    match k {
                        Expr(_, _, Expr_::String(s)) => {
                            Ok((alloc.alloc_slice_copy(&s) as &[u8], emit_expr(e, env, v)?))
                        }
                        _ => Err(Error::unrecoverable("Key must be a string")),
                    }
                }
            },
            _ => Err(Error::unrecoverable("impossible")),
        })
        .collect::<Result<Vec<_>>>()?
        .into_iter()
        .unzip();
    Ok(InstrSeq::gather(vec![
        InstrSeq::gather(value_instrs),
        emit_pos(pos),
        instr::new_struct_dict(&keys),
    ]))
}

fn emit_dynamic_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    fields: &[ast::Afield],
) -> Result<InstrSeq<'arena>> {
    let pos = &expr.1;
    let count = fields.len() as u32;
    let emit_dict = |e: &mut Emitter<'arena, 'decl>| {
        if is_struct_init(e, env, fields, true)? {
            emit_struct_array(e, env, pos, fields)
        } else {
            let ctor = Instruct::Opcode(Opcode::NewDictArray(count));
            emit_array(e, env, pos, fields, ctor)
        }
    };
    let emit_collection_helper = |e: &mut Emitter<'arena, 'decl>, ctype| {
        if is_struct_init(e, env, fields, true)? {
            Ok(InstrSeq::gather(vec![
                emit_struct_array(e, env, pos, fields)?,
                emit_pos(pos),
                instr::col_from_array(ctype),
            ]))
        } else {
            let ctor = Instruct::Opcode(Opcode::NewDictArray(count));
            emit_keyvalue_collection(e, env, pos, fields, ctype, ctor)
        }
    };
    use ast::Expr_;
    match &expr.2 {
        Expr_::ValCollection(v) if v.0.1 == ast::VcKind::Vec => {
            emit_value_only_collection(e, env, pos, fields, |v| Instruct::Opcode(Opcode::NewVec(v)))
        }
        Expr_::Tuple(_) => {
            emit_value_only_collection(e, env, pos, fields, |v| Instruct::Opcode(Opcode::NewVec(v)))
        }
        Expr_::ValCollection(v) if v.0.1 == ast::VcKind::Keyset => {
            emit_value_only_collection(e, env, pos, fields, |v| {
                Instruct::Opcode(Opcode::NewKeysetArray(v))
            })
        }
        Expr_::KeyValCollection(v) if v.0.1 == ast::KvcKind::Dict => emit_dict(e),
        Expr_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "Set" => {
            emit_collection_helper(e, CollectionType::Set)
        }
        Expr_::ValCollection(v) if v.0.1 == ast::VcKind::Set => {
            emit_collection_helper(e, CollectionType::Set)
        }
        Expr_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "ImmSet" => {
            emit_collection_helper(e, CollectionType::ImmSet)
        }
        Expr_::ValCollection(v) if v.0.1 == ast::VcKind::ImmSet => {
            emit_collection_helper(e, CollectionType::ImmSet)
        }
        Expr_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "Map" => {
            emit_collection_helper(e, CollectionType::Map)
        }
        Expr_::KeyValCollection(v) if v.0.1 == ast::KvcKind::Map => {
            emit_collection_helper(e, CollectionType::Map)
        }
        Expr_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "ImmMap" => {
            emit_collection_helper(e, CollectionType::ImmMap)
        }
        Expr_::KeyValCollection(v) if v.0.1 == ast::KvcKind::ImmMap => {
            emit_collection_helper(e, CollectionType::ImmMap)
        }
        _ => Err(Error::unrecoverable(
            "plain PHP arrays cannot be constructed",
        )),
    }
}

fn emit_value_only_collection<'a, 'arena, 'decl, F: FnOnce(u32) -> Instruct<'arena>>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
    constructor: F,
) -> Result<InstrSeq<'arena>> {
    let limit = e.options().max_array_elem_size_on_the_stack;
    let inline =
        |e: &mut Emitter<'arena, 'decl>, exprs: &[ast::Afield]| -> Result<InstrSeq<'arena>> {
            let mut instrs = vec![];
            for expr in exprs.iter() {
                instrs.push(emit_expr(e, env, expr.value())?)
            }

            Ok(InstrSeq::gather(vec![
                InstrSeq::gather(instrs),
                emit_pos(pos),
                instr::instr(constructor(exprs.len() as u32)),
            ]))
        };
    let outofline =
        |e: &mut Emitter<'arena, 'decl>, exprs: &[ast::Afield]| -> Result<InstrSeq<'arena>> {
            let mut instrs = vec![];
            for expr in exprs.iter() {
                instrs.push(emit_expr(e, env, expr.value())?);
                instrs.push(instr::add_new_elem_c());
            }
            Ok(InstrSeq::gather(instrs))
        };
    let (x1, x2) = fields.split_at(std::cmp::min(fields.len(), limit));
    Ok(match (x1, x2) {
        ([], []) => instr::empty(),
        (_, []) => inline(e, x1)?,
        _ => {
            let outofline_instrs = outofline(e, x2)?;
            let inline_instrs = inline(e, x1)?;
            InstrSeq::gather(vec![inline_instrs, outofline_instrs])
        }
    })
}

fn emit_call_isset_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    pk: &ParamKind,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    if pk.is_pinout() {
        return Err(Error::fatal_parse(
            outer_pos,
            "`isset` cannot take an argument by `inout`",
        ));
    }
    let alloc = env.arena;
    let pos = &expr.1;
    if let Some((base_expr, opt_elem_expr)) = expr.2.as_array_get() {
        return Ok(emit_array_get(
            e,
            env,
            pos,
            None,
            QueryMOp::Isset,
            base_expr,
            opt_elem_expr.as_ref(),
            false,
            false,
        )?
        .0);
    }
    if let Some((cid, id, _)) = expr.2.as_class_get() {
        return emit_class_get(e, env, QueryMOp::Isset, cid, id, ReadonlyOp::Any);
    }
    if let Some((expr_, prop, nullflavor, _)) = expr.2.as_obj_get() {
        return Ok(emit_obj_get(
            e,
            env,
            pos,
            QueryMOp::Isset,
            expr_,
            prop,
            nullflavor,
            false,
            false,
        )?
        .0);
    }
    if let Some(lid) = expr.2.as_lvar() {
        let name = local_id::get_name(&lid.1);
        return Ok(if superglobals::is_any_global(name) {
            InstrSeq::gather(vec![
                emit_pos(outer_pos),
                instr::string(alloc, string_utils::locals::strip_dollar(name)),
                emit_pos(outer_pos),
                instr::isset_g(),
            ])
        } else if is_local_this(env, &lid.1) && !env.flags.contains(env::Flags::NEEDS_LOCAL_THIS) {
            InstrSeq::gather(vec![
                emit_pos(outer_pos),
                emit_local(e, env, BareThisOp::NoNotice, lid)?,
                emit_pos(outer_pos),
                instr::is_type_c(IsTypeOp::Null),
                instr::not(),
            ])
        } else {
            emit_pos_then(outer_pos, instr::isset_l(get_local(e, env, &lid.0, name)?))
        });
    }
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        instr::is_type_c(IsTypeOp::Null),
        instr::not(),
    ]))
}

fn emit_call_isset_exprs<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    exprs: &[(ParamKind, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    match exprs {
        [] => Err(Error::fatal_parse(
            pos,
            "Cannot use isset() without any arguments",
        )),
        [(pk, expr)] => emit_call_isset_expr(e, env, pos, pk, expr),
        _ => {
            let its_done = e.label_gen_mut().next_regular();
            Ok(InstrSeq::gather(vec![
                InstrSeq::gather(
                    exprs
                        .iter()
                        .enumerate()
                        .map(|(i, (pk, expr))| {
                            Ok(InstrSeq::gather(vec![
                                emit_call_isset_expr(e, env, pos, pk, expr)?,
                                if i < exprs.len() - 1 {
                                    InstrSeq::gather(vec![
                                        instr::dup(),
                                        instr::jmp_z(its_done),
                                        instr::pop_c(),
                                    ])
                                } else {
                                    instr::empty()
                                },
                            ]))
                        })
                        .collect::<Result<Vec<_>>>()?,
                ),
                instr::label(its_done),
            ]))
        }
    }
}

fn emit_tag_provenance_here<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    es: &[(ParamKind, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    let pop = if es.len() == 1 {
        instr::empty()
    } else {
        instr::pop_c()
    };
    Ok(InstrSeq::gather(vec![
        emit_exprs_and_error_on_inout(e, env, es, "HH\\tag_provenance_here")?,
        emit_pos(pos),
        pop,
    ]))
}

fn emit_array_mark_legacy<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    es: &[(ParamKind, ast::Expr)],
    legacy: bool,
) -> Result<InstrSeq<'arena>> {
    let default = if es.len() == 1 {
        instr::false_()
    } else {
        instr::empty()
    };
    let mark = if legacy {
        instr::instr(Instruct::Opcode(Opcode::ArrayMarkLegacy))
    } else {
        instr::instr(Instruct::Opcode(Opcode::ArrayUnmarkLegacy))
    };
    Ok(InstrSeq::gather(vec![
        emit_exprs_and_error_on_inout(e, env, es, "HH\\array_mark_legacy")?,
        emit_pos(pos),
        default,
        mark,
    ]))
}

fn emit_idx<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    es: &[(ParamKind, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    let default = if es.len() == 2 {
        instr::null()
    } else {
        instr::empty()
    };
    Ok(InstrSeq::gather(vec![
        emit_exprs_and_error_on_inout(e, env, es, "idx")?,
        emit_pos(pos),
        default,
        instr::idx(),
    ]))
}

fn emit_call<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
    targs: &[ast::Targ],
    args: &[(ParamKind, ast::Expr)],
    uarg: Option<&ast::Expr>,
    async_eager_label: Option<Label>,
    readonly_return: bool,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    if let Some(ast_defs::Id(_, s)) = expr.as_id() {
        let fid = hhbc::FunctionName::<'arena>::from_ast_name(alloc, s);
        e.add_function_ref(fid);
    }
    let readonly_this = match &expr.2 {
        ast::Expr_::ReadonlyExpr(_) => true,
        _ => false,
    };
    let fcall_args = get_fcall_args(
        alloc,
        args,
        uarg,
        async_eager_label,
        env.call_context.clone(),
        false,
        readonly_return,
        readonly_this,
    );
    match expr.2.as_id() {
        None => emit_call_default(e, env, pos, expr, targs, args, uarg, fcall_args),
        Some(ast_defs::Id(_, id)) => {
            let fq = hhbc::FunctionName::<'arena>::from_ast_name(alloc, id);
            let lower_fq_name = fq.unsafe_as_str();
            emit_special_function(e, env, pos, targs, args, uarg, lower_fq_name)
                .transpose()
                .unwrap_or_else(|| {
                    emit_call_default(e, env, pos, expr, targs, args, uarg, fcall_args)
                })
        }
    }
}

fn emit_call_default<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
    targs: &[ast::Targ],
    args: &[(ParamKind, ast::Expr)],
    uarg: Option<&ast::Expr>,
    fcall_args: FCallArgs<'arena>,
) -> Result<InstrSeq<'arena>> {
    scope::with_unnamed_locals(e, |em| {
        let FCallArgs { num_rets, .. } = &fcall_args;
        let num_uninit = num_rets - 1;
        let (lhs, fcall) = emit_call_lhs_and_fcall(em, env, expr, fcall_args, targs, None)?;
        let (args, inout_setters) = emit_args_inout_setters(em, env, args)?;
        let uargs = match uarg {
            Some(uarg) => emit_expr(em, env, uarg)?,
            None => instr::empty(),
        };
        Ok((
            instr::empty(),
            InstrSeq::gather(vec![
                InstrSeq::gather(
                    iter::repeat_with(instr::null_uninit)
                        .take(num_uninit as usize)
                        .collect_vec(),
                ),
                lhs,
                args,
                uargs,
                emit_pos(pos),
                fcall,
                inout_setters,
            ]),
            instr::empty(),
        ))
    })
}

fn is_soft(ual: &[ast::UserAttribute]) -> bool {
    ual.iter().any(|ua| user_attributes::is_soft(&ua.name.1))
}

pub fn emit_reified_targs<'a, 'arena, 'decl, I>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    targs: I,
) -> Result<InstrSeq<'arena>>
where
    I: Iterator<Item = &'a ast::Hint> + ExactSizeIterator + Clone,
{
    let alloc = env.arena;
    let current_fun_tparams = env.scope.get_fun_tparams();
    let current_cls_tparams = env.scope.get_class_tparams();
    let is_in_lambda = env.scope.is_in_lambda();
    fn same_as_targs<'a, I>(targs: I, tparams: &[ast::Tparam]) -> bool
    where
        I: Iterator<Item = &'a ast::Hint> + ExactSizeIterator + Clone,
    {
        tparams.len() == targs.len()
            && tparams.iter().zip(targs).all(|(tp, ta)| {
                ta.1.as_happly().map_or(false, |(id, hs)| {
                    id.1 == tp.name.1
                        && hs.is_empty()
                        && !is_soft(&tp.user_attributes)
                        && tp.reified.is_reified()
                })
            })
    }
    Ok(
        if !is_in_lambda && same_as_targs(targs.clone(), current_fun_tparams) {
            instr::c_get_l(e.named_local(string_utils::reified::GENERICS_LOCAL_NAME.into()))
        } else if !is_in_lambda && same_as_targs(targs.clone(), current_cls_tparams) {
            InstrSeq::gather(vec![
                instr::check_this(),
                instr::base_h(),
                instr::query_m(
                    0,
                    QueryMOp::CGet,
                    MemberKey::PT(
                        hhbc::PropName::from_raw_string(alloc, string_utils::reified::PROP_NAME),
                        ReadonlyOp::Any,
                    ),
                ),
            ])
        } else {
            let targs_len = targs.len() as u32;
            InstrSeq::gather(vec![
                InstrSeq::gather(
                    targs
                        .map(|h| Ok(emit_reified_arg(e, env, pos, false, h)?.0))
                        .collect::<Result<Vec<_>>>()?,
                ),
                instr::new_vec(targs_len),
            ])
        },
    )
}

fn get_erased_tparams<'a, 'arena>(env: &'a Env<'a, 'arena>) -> Vec<&'a str> {
    env.scope
        .get_tparams()
        .iter()
        .filter_map(|tparam| match tparam.reified {
            ast::ReifyKind::Erased => Some(tparam.name.1.as_str()),
            _ => None,
        })
        .collect()
}

pub fn has_non_tparam_generics(env: &Env<'_, '_>, hints: &[ast::Hint]) -> bool {
    let erased_tparams = get_erased_tparams(env);
    hints.iter().any(|hint| {
        hint.1
            .as_happly()
            .map_or(true, |(id, _)| !erased_tparams.contains(&id.1.as_str()))
    })
}

fn has_non_tparam_generics_targs(env: &Env<'_, '_>, targs: &[ast::Targ]) -> bool {
    let erased_tparams = get_erased_tparams(env);
    targs.iter().any(|targ| {
        (targ.1)
            .1
            .as_happly()
            .map_or(true, |(id, _)| !erased_tparams.contains(&id.1.as_str()))
    })
}

fn from_ast_null_flavor(nullflavor: ast::OgNullFlavor) -> ObjMethodOp {
    match nullflavor {
        ast::OgNullFlavor::OGNullsafe => ObjMethodOp::NullSafe,
        ast::OgNullFlavor::OGNullthrows => ObjMethodOp::NullThrows,
    }
}

fn emit_object_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    match &expr.2 {
        ast::Expr_::Lvar(x) if is_local_this(env, &x.1) => Ok(instr::this()),
        _ => emit_expr(e, env, expr),
    }
}

fn emit_call_lhs_and_fcall<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    mut fcall_args: FCallArgs<'arena>,
    targs: &[ast::Targ],
    caller_readonly_opt: Option<&Pos>,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    let ast::Expr(_, pos, expr_) = expr;
    use ast::Expr;
    use ast::Expr_;
    let alloc = env.arena;
    let emit_generics =
        |e: &mut Emitter<'arena, 'decl>, env, fcall_args: &mut FCallArgs<'arena>| {
            let does_not_have_non_tparam_generics = !has_non_tparam_generics_targs(env, targs);
            if does_not_have_non_tparam_generics {
                Ok(instr::empty())
            } else {
                fcall_args.flags |= FCallArgsFlags::HasGenerics;
                emit_reified_targs(e, env, pos, targs.iter().map(|targ| &targ.1))
            }
        };

    let emit_fcall_func = |e: &mut Emitter<'arena, 'decl>,
                           env,
                           expr: &ast::Expr,
                           fcall_args: FCallArgs<'arena>,
                           caller_readonly_opt: Option<&Pos>|
     -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
        let tmp = e.local_gen_mut().get_unnamed();
        // if the original expression was wrapped in readonly, emit a readonly expression here
        let res = if let Some(p) = caller_readonly_opt {
            emit_readonly_expr(e, env, p, expr)?
        } else {
            emit_expr(e, env, expr)?
        };
        Ok((
            InstrSeq::gather(vec![
                instr::null_uninit(),
                instr::null_uninit(),
                res,
                instr::pop_l(tmp),
            ]),
            InstrSeq::gather(vec![instr::push_l(tmp), instr::f_call_func(fcall_args)]),
        ))
    };

    match expr_ {
        Expr_::ReadonlyExpr(r) => {
            // If calling a Readonly expression, first recurse inside to
            // handle ObjGet and ClassGet prop call cases. Keep track of the position of the
            // outer readonly expression for use later.
            // TODO: use the fact that this is a readonly call in HHVM enforcement
            emit_call_lhs_and_fcall(e, env, r, fcall_args, targs, Some(pos))
        }
        Expr_::ObjGet(o) if o.as_ref().3 == ast::PropOrMethod::IsMethod => {
            // Case $x->foo(...).
            // TODO: utilize caller_readonly_opt here for method calls
            let emit_id = |e: &mut Emitter<'arena, 'decl>,
                           obj,
                           id,
                           null_flavor: &ast::OgNullFlavor,
                           mut fcall_args| {
                let name =
                    hhbc::MethodName::new(Str::new_str(alloc, string_utils::strip_global_ns(id)));
                let obj = emit_object_expr(e, env, obj)?;
                let generics = emit_generics(e, env, &mut fcall_args)?;
                let null_flavor = from_ast_null_flavor(*null_flavor);
                Ok((
                    InstrSeq::gather(vec![obj, instr::null_uninit()]),
                    InstrSeq::gather(vec![
                        generics,
                        instr::f_call_obj_method_d_(fcall_args, name, null_flavor),
                    ]),
                ))
            };
            match o.as_ref() {
                (obj, Expr(_, _, Expr_::String(id)), null_flavor, _) => {
                    emit_id(
                        e,
                        obj,
                        // FIXME: This is not safe--string literals are binary strings.
                        // There's no guarantee that they're valid UTF-8.
                        unsafe { std::str::from_utf8_unchecked(id.as_slice()) },
                        null_flavor,
                        fcall_args,
                    )
                }
                (Expr(_, pos, Expr_::New(new_exp)), Expr(_, _, Expr_::Id(id)), null_flavor, _)
                    if fcall_args.num_args == 0 =>
                {
                    let cexpr =
                        ClassExpr::class_id_to_class_expr(e, &env.scope, false, false, &new_exp.0);
                    match &cexpr {
                        ClassExpr::Id(ast_defs::Id(_, name))
                            if string_utils::strip_global_ns(name) == "ReflectionClass" =>
                        {
                            let fid = match string_utils::strip_global_ns(&id.1) {
                                "isAbstract" => Some("__SystemLib\\reflection_class_is_abstract"),
                                "isInterface" => Some("__SystemLib\\reflection_class_is_interface"),
                                "isFinal" => Some("__SystemLib\\reflection_class_is_final"),
                                "getName" => Some("__SystemLib\\reflection_class_get_name"),
                                _ => None,
                            };
                            match fid {
                                None => emit_id(e, &o.as_ref().0, &id.1, null_flavor, fcall_args),
                                Some(fid) => {
                                    let fcall_args = FCallArgs::new(
                                        FCallArgsFlags::default(),
                                        1,
                                        1,
                                        vec![],
                                        vec![],
                                        None,
                                        None,
                                    );
                                    let newobj_instrs = emit_new(e, env, pos, new_exp, true);
                                    Ok((
                                        InstrSeq::gather(vec![
                                            instr::null_uninit(),
                                            instr::null_uninit(),
                                            newobj_instrs?,
                                        ]),
                                        InstrSeq::gather(vec![instr::f_call_func_d(
                                            fcall_args,
                                            hhbc::FunctionName::<'arena>::from_ast_name(alloc, fid),
                                        )]),
                                    ))
                                }
                            }
                        }
                        _ => emit_id(e, &o.as_ref().0, &id.1, null_flavor, fcall_args),
                    }
                }
                (obj, Expr(_, _, Expr_::Id(id)), null_flavor, _) => {
                    emit_id(e, obj, &id.1, null_flavor, fcall_args)
                }
                (obj, method_expr, null_flavor, _) => {
                    let obj = emit_object_expr(e, env, obj)?;
                    let tmp = e.local_gen_mut().get_unnamed();
                    let null_flavor = from_ast_null_flavor(*null_flavor);
                    Ok((
                        InstrSeq::gather(vec![
                            obj,
                            instr::null_uninit(),
                            emit_expr(e, env, method_expr)?,
                            instr::pop_l(tmp),
                        ]),
                        InstrSeq::gather(vec![
                            instr::push_l(tmp),
                            instr::f_call_obj_method(fcall_args, null_flavor),
                        ]),
                    ))
                }
            }
        }
        Expr_::ClassConst(cls_const) => {
            let (cid, (_, id)) = &**cls_const;
            let cexpr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, false, cid);
            let method_name =
                hhbc::MethodName::new(Str::new_str(alloc, string_utils::strip_global_ns(id)));
            Ok(match cexpr {
                // Statically known
                ClassExpr::Id(ast_defs::Id(_, cname)) => {
                    let cid = hhbc::ClassName::<'arena>::from_ast_name_and_mangle(alloc, &cname);
                    e.add_class_ref(cid.clone());
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(vec![instr::null_uninit(), instr::null_uninit()]),
                        InstrSeq::gather(vec![
                            generics,
                            instr::f_call_cls_method_d(fcall_args, method_name, cid),
                        ]),
                    )
                }
                ClassExpr::Special(clsref) => {
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(vec![instr::null_uninit(), instr::null_uninit()]),
                        InstrSeq::gather(vec![
                            generics,
                            instr::f_call_cls_method_sd(fcall_args, clsref, method_name),
                        ]),
                    )
                }
                ClassExpr::Expr(expr) => {
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(vec![instr::null_uninit(), instr::null_uninit()]),
                        InstrSeq::gather(vec![
                            generics,
                            emit_expr(e, env, &expr)?,
                            instr::f_call_cls_method_m(
                                IsLogAsDynamicCallOp::DontLogAsDynamicCall,
                                fcall_args,
                                method_name,
                            ),
                        ]),
                    )
                }
                ClassExpr::Reified(ast::Id(pos, name)) => {
                    let tmp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            instr::null_uninit(),
                            instr::null_uninit(),
                            get_reified_var_cexpr(e, env, &pos, &name)?,
                            instr::pop_l(tmp),
                        ]),
                        InstrSeq::gather(vec![
                            instr::push_l(tmp),
                            instr::f_call_cls_method_m(
                                IsLogAsDynamicCallOp::DontLogAsDynamicCall,
                                fcall_args,
                                method_name,
                            ),
                        ]),
                    )
                }
            })
        }
        Expr_::ClassGet(c) if c.as_ref().2 == ast::PropOrMethod::IsMethod => {
            // Case Foo::bar(...).
            let (cid, cls_get_expr, _) = &**c;
            let cexpr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, false, cid);
            let emit_meth_name = |e: &mut Emitter<'arena, 'decl>| match &cls_get_expr {
                ast::ClassGetExpr::CGstring((pos, id)) => {
                    Ok(emit_pos_then(pos, instr::c_get_l(e.named_local(id.into()))))
                }
                ast::ClassGetExpr::CGexpr(expr) => emit_expr(e, env, expr),
            };
            Ok(match cexpr {
                ClassExpr::Id(cid) => {
                    let tmp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            instr::null_uninit(),
                            instr::null_uninit(),
                            emit_meth_name(e)?,
                            instr::pop_l(tmp),
                        ]),
                        InstrSeq::gather(vec![
                            instr::push_l(tmp),
                            emit_known_class_id(alloc, e, &cid),
                            instr::f_call_cls_method(
                                IsLogAsDynamicCallOp::LogAsDynamicCall,
                                fcall_args,
                            ),
                        ]),
                    )
                }
                ClassExpr::Special(clsref) => {
                    let tmp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            instr::null_uninit(),
                            instr::null_uninit(),
                            emit_meth_name(e)?,
                            instr::pop_l(tmp),
                        ]),
                        InstrSeq::gather(vec![
                            instr::push_l(tmp),
                            instr::f_call_cls_method_s(fcall_args, clsref),
                        ]),
                    )
                }
                ClassExpr::Expr(expr) => {
                    let cls = e.local_gen_mut().get_unnamed();
                    let meth = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            instr::null_uninit(),
                            instr::null_uninit(),
                            emit_expr(e, env, &expr)?,
                            instr::pop_l(cls),
                            emit_meth_name(e)?,
                            instr::pop_l(meth),
                        ]),
                        InstrSeq::gather(vec![
                            instr::push_l(meth),
                            instr::push_l(cls),
                            instr::class_get_c(ClassGetCMode::Normal),
                            instr::f_call_cls_method(
                                IsLogAsDynamicCallOp::LogAsDynamicCall,
                                fcall_args,
                            ),
                        ]),
                    )
                }
                ClassExpr::Reified(ast::Id(pos, name)) => {
                    let cls = e.local_gen_mut().get_unnamed();
                    let meth = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            instr::null_uninit(),
                            instr::null_uninit(),
                            get_reified_var_cexpr(e, env, &pos, &name)?,
                            instr::pop_l(cls),
                            emit_meth_name(e)?,
                            instr::pop_l(meth),
                        ]),
                        InstrSeq::gather(vec![
                            instr::push_l(meth),
                            instr::push_l(cls),
                            instr::class_get_c(ClassGetCMode::Normal),
                            instr::f_call_cls_method(
                                IsLogAsDynamicCallOp::LogAsDynamicCall,
                                fcall_args,
                            ),
                        ]),
                    )
                }
            })
        }
        Expr_::Id(id) => {
            let FCallArgs {
                flags, num_args, ..
            } = fcall_args;
            let fq_id = match string_utils::strip_global_ns(&id.1) {
                "min" if num_args == 2 && !flags.contains(FCallArgsFlags::HasUnpack) => {
                    hhbc::FunctionName::<'arena>::from_ast_name(alloc, "__SystemLib\\min2")
                }
                "max" if num_args == 2 && !flags.contains(FCallArgsFlags::HasUnpack) => {
                    hhbc::FunctionName::<'arena>::from_ast_name(alloc, "__SystemLib\\max2")
                }
                _ => hhbc::FunctionName::new(Str::new_str(
                    alloc,
                    string_utils::strip_global_ns(&id.1),
                )),
            };
            let generics = emit_generics(e, env, &mut fcall_args)?;
            Ok((
                InstrSeq::gather(vec![instr::null_uninit(), instr::null_uninit()]),
                InstrSeq::gather(vec![generics, instr::f_call_func_d(fcall_args, fq_id)]),
            ))
        }
        Expr_::String(s) => {
            // TODO(hrust) should be able to accept `let fq_id = function::from_raw_string(s);`
            let fq_id = hhbc::FunctionName::new(Str::new_str(alloc, s.to_string().as_str()));
            let generics = emit_generics(e, env, &mut fcall_args)?;
            Ok((
                InstrSeq::gather(vec![instr::null_uninit(), instr::null_uninit()]),
                InstrSeq::gather(vec![generics, instr::f_call_func_d(fcall_args, fq_id)]),
            ))
        }
        _ => emit_fcall_func(e, env, expr, fcall_args, caller_readonly_opt),
    }
}

fn get_reified_var_cexpr<'a, 'arena>(
    e: &Emitter<'arena, '_>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    name: &str,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![
        emit_reified_type(e, env, pos, name)?,
        instr::base_c(0, MOpMode::Warn),
        instr::query_m(
            1,
            QueryMOp::CGet,
            MemberKey::ET(Str::from("classname"), ReadonlyOp::Any),
        ),
    ]))
}

fn emit_args_inout_setters<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    args: &[(ParamKind, ast::Expr)],
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    let aliases = if has_inout_arg(args) {
        inout_locals::collect_written_variables(env, args)
    } else {
        inout_locals::new_alias_info_map()
    };
    fn emit_arg_and_inout_setter<'a, 'arena, 'decl>(
        e: &mut Emitter<'arena, 'decl>,
        env: &Env<'a, 'arena>,
        i: usize,
        pk: &ParamKind,
        arg: &ast::Expr,
        aliases: &inout_locals::AliasInfoMap<'_>,
    ) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
        use ast::Expr_;

        match (pk, &arg.2) {
            // inout $var
            (ParamKind::Pinout(_), Expr_::Lvar(l)) => {
                let local = get_local(e, env, &l.0, local_id::get_name(&l.1))?;
                let move_instrs = if !env.flags.contains(env::Flags::IN_TRY)
                    && inout_locals::should_move_local_value(e, local, aliases)
                {
                    InstrSeq::gather(vec![instr::null(), instr::pop_l(local)])
                } else {
                    instr::empty()
                };
                Ok((
                    InstrSeq::gather(vec![instr::c_get_l(local), move_instrs]),
                    instr::pop_l(local),
                ))
            }
            // inout $arr[...][...]
            (ParamKind::Pinout(_), Expr_::ArrayGet(ag)) => {
                let array_get_result = emit_array_get_(
                    e,
                    env,
                    &arg.1,
                    None,
                    QueryMOp::InOut,
                    &ag.0,
                    ag.1.as_ref(),
                    false,
                    false,
                    Some((i, aliases)),
                )?
                .0;
                Ok(match array_get_result {
                    ArrayGetInstr::Regular(instrs) => {
                        let setter_base = emit_array_get(
                            e,
                            env,
                            &arg.1,
                            Some(MOpMode::Define),
                            QueryMOp::InOut,
                            &ag.0,
                            ag.1.as_ref(),
                            true,
                            false,
                        )?
                        .0;
                        let (mk, warninstr) = get_elem_member_key(e, env, 0, ag.1.as_ref(), false)?;
                        let setter = InstrSeq::gather(vec![
                            warninstr,
                            setter_base,
                            instr::set_m(0, mk),
                            instr::pop_c(),
                        ]);
                        (instrs, setter)
                    }
                    ArrayGetInstr::Inout { load, store } => {
                        let (mut ld, mut st) = (vec![], vec![store]);
                        for (instr, local_kind_opt) in load.into_iter() {
                            match local_kind_opt {
                                None => ld.push(instr),
                                Some((l, kind)) => {
                                    let unset = instr::unset_l(l);
                                    let set = match kind {
                                        StoredValueKind::Expr => instr::set_l(l),
                                        _ => instr::pop_l(l),
                                    };
                                    ld.push(instr);
                                    ld.push(set);
                                    st.push(unset);
                                }
                            }
                        }
                        (InstrSeq::gather(ld), InstrSeq::gather(st))
                    }
                })
            }
            (ParamKind::Pinout(_), _) => Err(Error::unrecoverable(
                "emit_arg_and_inout_setter: Unexpected inout expression type",
            )),
            _ => Ok((emit_expr(e, env, arg)?, instr::empty())),
        }
    }
    let (instr_args, instr_setters): (Vec<InstrSeq<'_>>, Vec<InstrSeq<'_>>) = args
        .iter()
        .enumerate()
        .map(|(i, (pk, arg))| emit_arg_and_inout_setter(e, env, i, pk, arg, &aliases))
        .collect::<Result<Vec<_>>>()?
        .into_iter()
        .unzip();
    let instr_args = InstrSeq::gather(instr_args);
    let instr_setters = InstrSeq::gather(instr_setters);
    if has_inout_arg(args) {
        let retval = e.local_gen_mut().get_unnamed();
        Ok((
            instr_args,
            InstrSeq::gather(vec![
                instr::pop_l(retval),
                instr_setters,
                instr::push_l(retval),
            ]),
        ))
    } else {
        Ok((instr_args, instr::empty()))
    }
}

/// You can think of Hack as having two "types" of function calls:
/// - Normal function calls, where the parameters might have non-standard calling convention
/// - "Special calls" where we expect all parameters to be passed in normally, mainly (if not
///   exclusively) object constructors
///
/// This function abstracts over these two kinds of calls: given a list of arguments and two
/// predicates (is this an `inout` argument, is this `readonly`) we build up an `FCallArgs` for the
/// given function call.
fn get_fcall_args_common<'arena, T>(
    alloc: &'arena bumpalo::Bump,
    args: &[T],
    uarg: Option<&ast::Expr>,
    async_eager_label: Option<Label>,
    context: Option<String>,
    lock_while_unwinding: bool,
    readonly_return: bool,
    readonly_this: bool,
    readonly_predicate: fn(&T) -> bool,
    is_inout_arg: fn(&T) -> bool,
) -> FCallArgs<'arena> {
    let mut flags = FCallArgsFlags::default();
    flags.set(FCallArgsFlags::HasUnpack, uarg.is_some());
    flags.set(FCallArgsFlags::LockWhileUnwinding, lock_while_unwinding);
    flags.set(FCallArgsFlags::EnforceMutableReturn, !readonly_return);
    flags.set(FCallArgsFlags::EnforceReadonlyThis, readonly_this);
    let readonly_args = if args.iter().any(readonly_predicate) {
        args.iter().map(readonly_predicate).collect()
    } else {
        vec![]
    };
    FCallArgs::new(
        flags,
        1 + args.iter().filter(|e| is_inout_arg(e)).count() as u32,
        args.len() as u32,
        args.iter().map(is_inout_arg).collect(),
        readonly_args,
        async_eager_label,
        context
            .map(|s| bumpalo::collections::String::from_str_in(s.as_str(), alloc).into_bump_str()),
    )
}

fn get_fcall_args_no_inout<'arena>(
    alloc: &'arena bumpalo::Bump,
    args: &[ast::Expr],
    uarg: Option<&ast::Expr>,
    async_eager_label: Option<Label>,
    context: Option<String>,
    lock_while_unwinding: bool,
    readonly_return: bool,
    readonly_this: bool,
) -> FCallArgs<'arena> {
    get_fcall_args_common(
        alloc,
        args,
        uarg,
        async_eager_label,
        context,
        lock_while_unwinding,
        readonly_return,
        readonly_this,
        is_readonly_expr,
        |_| false,
    )
}

fn get_fcall_args<'arena>(
    alloc: &'arena bumpalo::Bump,
    args: &[(ParamKind, ast::Expr)],
    uarg: Option<&ast::Expr>,
    async_eager_label: Option<Label>,
    context: Option<String>,
    lock_while_unwinding: bool,
    readonly_return: bool,
    readonly_this: bool,
) -> FCallArgs<'arena> {
    get_fcall_args_common(
        alloc,
        args,
        uarg,
        async_eager_label,
        context,
        lock_while_unwinding,
        readonly_return,
        readonly_this,
        |(_, expr): &(ParamKind, ast::Expr)| is_readonly_expr(expr),
        |(pk, _): &(ParamKind, ast::Expr)| pk.is_pinout(),
    )
}

fn is_readonly_expr(e: &ast::Expr) -> bool {
    match &e.2 {
        ast::Expr_::ReadonlyExpr(_) => true,
        _ => false,
    }
}

fn has_inout_arg(es: &[(ParamKind, ast::Expr)]) -> bool {
    es.iter().any(|(pk, _)| pk.is_pinout())
}

fn emit_special_function<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    targs: &[ast::Targ],
    args: &[(ParamKind, ast::Expr)],
    uarg: Option<&ast::Expr>,
    lower_fq_name: &str,
) -> Result<Option<InstrSeq<'arena>>> {
    use ast::Expr;
    use ast::Expr_;
    let alloc = env.arena;
    let nargs = args.len() + uarg.map_or(0, |_| 1);
    match (lower_fq_name, args) {
        (id, _) if id == special_functions::ECHO => Ok(Some(InstrSeq::gather(
            args.iter()
                .enumerate()
                .map(|(i, arg)| {
                    Ok(InstrSeq::gather(vec![
                        emit_expr(e, env, error::expect_normal_paramkind(arg)?)?,
                        emit_pos(pos),
                        instr::print(),
                        if i == nargs - 1 {
                            instr::empty()
                        } else {
                            instr::pop_c()
                        },
                    ]))
                })
                .collect::<Result<_>>()?,
        ))),
        ("HH\\invariant", args) if args.len() >= 2 => {
            let l = e.label_gen_mut().next_regular();
            let expr_id = ast::Expr(
                (),
                pos.clone(),
                ast::Expr_::mk_id(ast_defs::Id(
                    pos.clone(),
                    "\\HH\\invariant_violation".into(),
                )),
            );
            let call = ast::Expr(
                (),
                pos.clone(),
                ast::Expr_::mk_call(ast::CallExpr { func: expr_id, targs: vec![], args: args[1..].to_owned(), unpacked_arg: uarg.cloned() }),
            );
            let ignored_expr = emit_ignored_expr(e, env, &Pos::NONE, &call)?;
            Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, error::expect_normal_paramkind(&args[0])?)?,
                instr::jmp_nz(l),
                ignored_expr,
                emit_fatal::emit_fatal_runtime(alloc, pos, "invariant_violation"),
                instr::label(l),
                instr::null(),
            ])))
        }
        ("class_exists", &[ref arg1, ..])
        | ("trait_exists", &[ref arg1, ..])
        | ("interface_exists", &[ref arg1, ..])
            if nargs == 1 || nargs == 2 =>
        {
            let class_kind = match lower_fq_name {
                "class_exists" => OODeclExistsOp::Class,
                "interface_exists" => OODeclExistsOp::Interface,
                "trait_exists" => OODeclExistsOp::Trait,
                _ => return Err(Error::unrecoverable("emit_special_function: class_kind")),
            };
            Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, error::expect_normal_paramkind(arg1)?)?,
                instr::cast_string(),
                if nargs == 1 {
                    instr::true_()
                } else {
                    InstrSeq::gather(vec![
                        emit_expr(e, env, error::expect_normal_paramkind(&args[1])?)?,
                        instr::cast_bool(),
                    ])
                },
                instr::oo_decl_exists(class_kind),
            ])))
        }
        ("exit", _) if nargs == 0 || nargs == 1 => Ok(Some(emit_exit(
            e,
            env,
            args.first()
                .map(error::expect_normal_paramkind)
                .transpose()?,
        )?)),
        ("__SystemLib\\meth_caller", _) => {
            // used by meth_caller() to directly emit func ptr
            if nargs != 1 {
                return Err(Error::fatal_runtime(
                    pos,
                    format!("fun() expects exactly 1 parameter, {} given", nargs),
                ));
            }
            match args {
                // `inout` is dropped here, but it should be impossible to have an expression
                // like: `foo(inout "literal")`
                [(_, Expr(_, _, Expr_::String(ref func_name)))] => {
                    Ok(Some(instr::resolve_meth_caller(hhbc::FunctionName::new(
                        Str::new_str(
                            alloc,
                            string_utils::strip_global_ns(
                                // FIXME: This is not safe--string literals are binary strings.
                                // There's no guarantee that they're valid UTF-8.
                                unsafe { std::str::from_utf8_unchecked(func_name.as_slice()) },
                            ),
                        ),
                    ))))
                }
                _ => Err(Error::fatal_runtime(
                    pos,
                    "Constant string expected in fun()",
                )),
            }
        }
        ("__SystemLib\\__debugger_is_uninit", _) => {
            if nargs != 1 {
                Err(Error::fatal_runtime(
                    pos,
                    format!(
                        "__debugger_is_uninit() expects exactly 1 parameter {} given",
                        nargs
                    ),
                ))
            } else {
                match args {
                    // Calling convention is dropped here, but given this is meant for the debugger
                    // I don't think it particularly matters.
                    [(_, Expr(_, _, Expr_::Lvar(id)))] => {
                        Ok(Some(instr::is_unset_l(get_local(e, env, pos, id.name())?)))
                    }
                    _ => Err(Error::fatal_runtime(
                        pos,
                        "Local variable expected in __debugger_is_uninit()",
                    )),
                }
            }
        }
        ("__SystemLib\\get_enum_member_by_label", _) if e.systemlib() => {
            let local = match args {
                [(pk, Expr(_, _, Expr_::Lvar(id)))] => {
                    error::ensure_normal_paramkind(pk)?;
                    get_local(e, env, pos, id.name())
                }
                _ => Err(Error::fatal_runtime(
                    pos,
                    "Argument must be the label argument",
                )),
            }?;
            Ok(Some(InstrSeq::gather(vec![
                instr::late_bound_cls(),
                instr::cls_cns_l(local),
            ])))
        }
        ("__SystemLib\\unwrap_opaque_value", _) if e.options().hhbc.enable_native_enum_class_labels => match *args {
            [_, ref val] =>
            Ok(Some(InstrSeq::gather(vec![
                 emit_expr(e, env, error::expect_normal_paramkind(val)?)?,
                 emit_pos(pos),
                 instr::enum_class_label_name(),
            ]))),
            _ => Err(Error::fatal_runtime(
                pos,
                format!("__SystemLib\\unwrap_opaque_value() expects exactly 2 parameters, {} given", nargs),
            )),
        },
        ("HH\\classname_to_class", _) => match *args {
            [ref cname] => Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, error::expect_normal_paramkind(cname)?)?,
                instr::class_get_c(ClassGetCMode::ExplicitConversion),
            ]))),
            _ => Err(Error::fatal_runtime(
                pos,
                format!("classname_to_class() expects exactly 1 parameter, {} given", nargs),
            )),
        },
        ("HH\\global_set", _) => match *args {
            [ref gkey, ref gvalue] => Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, error::expect_normal_paramkind(gkey)?)?,
                emit_expr(e, env, error::expect_normal_paramkind(gvalue)?)?,
                emit_pos(pos),
                instr::set_g(),
                instr::pop_c(),
                instr::null(),
            ]))),
            _ => Err(Error::fatal_runtime(
                pos,
                format!("global_set() expects exactly 2 parameters, {} given", nargs),
            )),
        },
        ("HH\\global_unset", _) => match *args {
            [ref gkey] => Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, error::expect_normal_paramkind(gkey)?)?,
                emit_pos(pos),
                instr::unset_g(),
                instr::null(),
            ]))),
            _ => Err(Error::fatal_runtime(
                pos,
                format!(
                    "global_unset() expects exactly 1 parameter, {} given",
                    nargs
                ),
            )),
        },
        ("__hhvm_internal_whresult", &[(ref pk, Expr(_, _, Expr_::Lvar(ref param)))])
            if e.systemlib() =>
        {
            error::ensure_normal_paramkind(pk)?;
            Ok(Some(InstrSeq::gather(vec![
                instr::c_get_l(e.named_local(local_id::get_name(&param.1).into())),
                instr::wh_result(),
            ])))
        }
        ("HH\\array_mark_legacy", _) if args.len() == 1 || args.len() == 2 => {
            Ok(Some(emit_array_mark_legacy(e, env, pos, args, true)?))
        }
        ("HH\\array_unmark_legacy", _) if args.len() == 1 || args.len() == 2 => {
            Ok(Some(emit_array_mark_legacy(e, env, pos, args, false)?))
        }
        ("HH\\tag_provenance_here", _) if args.len() == 1 || args.len() == 2 => {
            Ok(Some(emit_tag_provenance_here(e, env, pos, args)?))
        }
        ("HH\\embed_type_decl", _)
            // `enable_intrinsics_extension` is roughly being used as a proxy here for "are we in
            // a non-production environment?" `embed_type_decl` is *not* fit for production use.
            // The typechecker doesn't understand it anyhow.
            if e.options().hhbc.enable_intrinsics_extension
                && args.is_empty()
                && targs.len() == 1
                && e.decl_provider.is_some() =>
        {
            match (targs[0].1).1.as_ref() {
                aast_defs::Hint_::Happly(ast_defs::Id(_, id), _) => {
                    let str = match e.decl_provider.as_ref().unwrap().type_decl(id, 0) {
                        Ok(decl_provider::TypeDecl::Class(cls)) => json!({
                            "name": cls.name.1,
                            "final": cls.final_,
                            "abstract": cls.abstract_,
                            "kind": (match cls.kind {
                                ast_defs::ClassishKind::Cclass(_) => "class",
                                ast_defs::ClassishKind::Cinterface => "interface",
                                ast_defs::ClassishKind::Ctrait => "trait",
                                ast_defs::ClassishKind::Cenum => "enum",
                                ast_defs::ClassishKind::CenumClass(_) => "enum class",
                            })
                        })
                        .to_string(),
                        Ok(decl_provider::TypeDecl::Typedef(td)) => json!({
                            "kind": (match td.vis {
                                aast_defs::TypedefVisibility::Transparent => "type",
                                aast_defs::TypedefVisibility::Opaque => "newtype",
                                aast_defs::TypedefVisibility::OpaqueModule => "module newtype",
                                aast_defs::TypedefVisibility::CaseType => "case type",
                            })
                        })
                        .to_string(),
                        Err(e) => {
                            let s = format!("Error: {}", e);
                            serde_json::to_string(&s).unwrap()
                        }
                    };
                    Ok(Some(instr::string(e.alloc, str)))
                }
                // If we don't have a classish type hint, compile this as
                // normal and let the call fail down the line (for now)
                _ => Ok(None),
            }
        }
        _ => Ok(
            match (args, istype_op(lower_fq_name), is_isexp_op(lower_fq_name)) {
                ([arg_expr], _, Some(ref h)) => {
                    let is_expr = emit_is(e, env, pos, h)?;
                    Some(InstrSeq::gather(vec![
                        emit_expr(e, env, error::expect_normal_paramkind(arg_expr)?)?,
                        is_expr,
                    ]))
                }
                (&[(ref pk, Expr(_, _, Expr_::Lvar(ref arg_id)))], Some(i), _)
                    if superglobals::is_any_global(arg_id.name()) =>
                {
                    error::ensure_normal_paramkind(pk)?;
                    Some(InstrSeq::gather(vec![
                        emit_local(e, env, BareThisOp::NoNotice, arg_id)?,
                        emit_pos(pos),
                        instr::is_type_c(i),
                    ]))
                }
                (&[(ref pk, Expr(_, _, Expr_::Lvar(ref arg_id)))], Some(i), _)
                    if !is_local_this(env, &arg_id.1) =>
                {
                    error::ensure_normal_paramkind(pk)?;
                    Some(instr::is_type_l(
                        get_local(e, env, &arg_id.0, &(arg_id.1).1)?,
                        i,
                    ))
                }
                (&[(ref pk, ref arg_expr)], Some(i), _) => {
                    error::ensure_normal_paramkind(pk)?;
                    Some(InstrSeq::gather(vec![
                        emit_expr(e, env, arg_expr)?,
                        emit_pos(pos),
                        instr::is_type_c(i),
                    ]))
                }
                _ => match get_call_builtin_func_info(e, lower_fq_name) {
                    Some((nargs, i)) if nargs == args.len() => Some(InstrSeq::gather(vec![
                        emit_exprs_and_error_on_inout(e, env, args, lower_fq_name)?,
                        emit_pos(pos),
                        instr::instr(i),
                    ])),
                    _ => None,
                },
            },
        ),
    }
}

fn emit_class_meth_native<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    cid: &ast::ClassId,
    method_name: MethodName<'arena>,
    targs: &[ast::Targ],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let cexpr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, true, cid);
    let has_generics = has_non_tparam_generics_targs(env, targs);
    let emit_generics = |e| -> Result<InstrSeq<'arena>> {
        emit_reified_targs(e, env, pos, targs.iter().map(|targ| &targ.1))
    };
    Ok(match cexpr {
        ClassExpr::Id(ast_defs::Id(_, name)) if !has_generics => instr::resolve_cls_method_d(
            hhbc::ClassName::<'arena>::from_ast_name_and_mangle(alloc, &name),
            method_name,
        ),
        ClassExpr::Id(ast_defs::Id(_, name)) => InstrSeq::gather(vec![
            emit_generics(e)?,
            instr::resolve_r_cls_method_d(
                hhbc::ClassName::<'arena>::from_ast_name_and_mangle(alloc, &name),
                method_name,
            ),
        ]),
        ClassExpr::Special(clsref) if !has_generics => {
            instr::resolve_cls_method_s(clsref, method_name)
        }
        ClassExpr::Special(clsref) => InstrSeq::gather(vec![
            emit_generics(e)?,
            instr::resolve_r_cls_method_s(clsref, method_name),
        ]),
        ClassExpr::Reified(ast_defs::Id(pos, name)) if !has_generics => InstrSeq::gather(vec![
            get_reified_var_cexpr(e, env, &pos, &name)?,
            instr::class_get_c(ClassGetCMode::Normal),
            instr::resolve_cls_method(method_name),
        ]),
        ClassExpr::Reified(ast_defs::Id(pos, name)) => InstrSeq::gather(vec![
            get_reified_var_cexpr(e, env, &pos, &name)?,
            instr::class_get_c(ClassGetCMode::Normal),
            emit_generics(e)?,
            instr::resolve_r_cls_method(method_name),
        ]),
        ClassExpr::Expr(_) => {
            return Err(Error::unrecoverable(
                "emit_class_meth_native: ClassExpr::Expr should be impossible",
            ));
        }
    })
}

fn get_call_builtin_func_info<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    id: impl AsRef<str>,
) -> Option<(usize, Instruct<'arena>)> {
    match id.as_ref() {
        "array_key_exists" => Some((2, Instruct::Opcode(Opcode::AKExists))),
        "hphp_array_idx" => Some((3, Instruct::Opcode(Opcode::ArrayIdx))),
        "intval" => Some((1, Instruct::Opcode(Opcode::CastInt))),
        "boolval" => Some((1, Instruct::Opcode(Opcode::CastBool))),
        "strval" => Some((1, Instruct::Opcode(Opcode::CastString))),
        "floatval" | "doubleval" => Some((1, Instruct::Opcode(Opcode::CastDouble))),
        "HH\\vec" => Some((1, Instruct::Opcode(Opcode::CastVec))),
        "HH\\keyset" => Some((1, Instruct::Opcode(Opcode::CastKeyset))),
        "HH\\dict" => Some((1, Instruct::Opcode(Opcode::CastDict))),
        "HH\\varray" => Some((1, Instruct::Opcode(Opcode::CastVec))),
        "HH\\darray" => Some((1, Instruct::Opcode(Opcode::CastDict))),
        "HH\\ImplicitContext\\_Private\\set_implicit_context_by_value" if e.systemlib() => {
            Some((1, Instruct::Opcode(Opcode::SetImplicitContextByValue)))
        }
        "HH\\ImplicitContext\\_Private\\create_special_implicit_context" if e.systemlib() => {
            Some((2, Instruct::Opcode(Opcode::CreateSpecialImplicitContext)))
        }
        // TODO: enforce that this returns readonly
        "HH\\global_readonly_get" => Some((1, Instruct::Opcode(Opcode::CGetG))),
        "HH\\global_get" => Some((1, Instruct::Opcode(Opcode::CGetG))),
        "HH\\global_isset" => Some((1, Instruct::Opcode(Opcode::IssetG))),
        _ => None,
    }
}

fn emit_function_pointer<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fpid: &ast::FunctionPtrId,
    targs: &[ast::Targ],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let instrs = match fpid {
        // This is a function name. Equivalent to HH\fun('str')
        ast::FunctionPtrId::FPId(id) => emit_hh_fun(e, env, pos, targs, id.name())?,
        // class_meth
        ast::FunctionPtrId::FPClassConst(cid, method_name) => {
            // TODO(hrust) should accept
            //   `let method_name = hhbc::MethodName::from_ast_name(&(cc.1).1);`
            let method_name = hhbc::MethodName::new(Str::new_str(
                alloc,
                string_utils::strip_global_ns(&method_name.1),
            ));
            emit_class_meth_native(e, env, pos, cid, method_name, targs)?
        }
    };
    Ok(emit_pos_then(pos, instrs))
}

fn emit_hh_fun<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    targs: &[ast::Targ],
    fname: &str,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let fname = string_utils::strip_global_ns(fname);
    if has_non_tparam_generics_targs(env, targs) {
        let generics = emit_reified_targs(e, env, pos, targs.iter().map(|targ| &targ.1))?;
        Ok(InstrSeq::gather(vec![
            generics,
            instr::resolve_r_func(hhbc::FunctionName::new(Str::new_str(alloc, fname))),
        ]))
    } else {
        Ok(instr::resolve_func(hhbc::FunctionName::new(Str::new_str(
            alloc, fname,
        ))))
    }
}

fn emit_is<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    h: &ast::Hint,
) -> Result<InstrSeq<'arena>> {
    emit_is_with_kind(e, env, pos, h, hhbc::TypeStructEnforceKind::Deep)
}

fn emit_is_shallow<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    h: &ast::Hint,
) -> Result<InstrSeq<'arena>> {
    emit_is_with_kind(e, env, pos, h, hhbc::TypeStructEnforceKind::Shallow)
}

fn emit_is_with_kind<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    h: &ast::Hint,
    kind: hhbc::TypeStructEnforceKind,
) -> Result<InstrSeq<'arena>> {
    let (ts_instrs, is_static) = emit_reified_arg(e, env, pos, true, h)?;
    Ok(if is_static {
        match &*h.1 {
            aast_defs::Hint_::Happly(ast_defs::Id(_, id), hs)
                if hs.is_empty() && string_utils::strip_hh_ns(id) == typehints::THIS =>
            {
                instr::is_late_bound_cls()
            }
            _ => InstrSeq::gather(vec![
                get_type_structure_for_hint(
                    e,
                    &[],
                    &IndexSet::new(),
                    TypeRefinementInHint::Disallowed,
                    h,
                )?,
                instr::is_type_struct_c(hhbc::TypeStructResolveOp::Resolve, kind),
            ]),
        }
    } else {
        InstrSeq::gather(vec![
            ts_instrs,
            instr::is_type_struct_c(hhbc::TypeStructResolveOp::DontResolve, kind),
        ])
    })
}

fn istype_op(id: impl AsRef<str>) -> Option<IsTypeOp> {
    match id.as_ref() {
        "is_int" | "is_integer" | "is_long" => Some(IsTypeOp::Int),
        "is_bool" => Some(IsTypeOp::Bool),
        "is_float" | "is_real" | "is_double" => Some(IsTypeOp::Dbl),
        "is_string" => Some(IsTypeOp::Str),
        "is_object" => Some(IsTypeOp::Obj),
        "is_null" => Some(IsTypeOp::Null),
        "is_scalar" => Some(IsTypeOp::Scalar),
        "HH\\is_keyset" => Some(IsTypeOp::Keyset),
        "HH\\is_dict" => Some(IsTypeOp::Dict),
        "HH\\is_vec" => Some(IsTypeOp::Vec),
        "HH\\is_varray" => Some(IsTypeOp::Vec),
        "HH\\is_darray" => Some(IsTypeOp::Dict),
        "HH\\is_any_array" => Some(IsTypeOp::ArrLike),
        "HH\\is_class_meth" => Some(IsTypeOp::ClsMeth),
        "HH\\is_fun" => Some(IsTypeOp::Func),
        "HH\\is_php_array" => Some(IsTypeOp::LegacyArrLike),
        "HH\\is_array_marked_legacy" => Some(IsTypeOp::LegacyArrLike),
        "HH\\is_class" => Some(IsTypeOp::Class),
        _ => None,
    }
}

fn is_isexp_op(lower_fq_id: impl AsRef<str>) -> Option<ast::Hint> {
    let h = |s: &str| {
        Some(ast::Hint::new(
            Pos::NONE,
            ast::Hint_::mk_happly(ast::Id(Pos::NONE, s.into()), vec![]),
        ))
    };
    match lower_fq_id.as_ref() {
        "is_int" | "is_integer" | "is_long" => h("\\HH\\int"),
        "is_bool" => h("\\HH\\bool"),
        "is_float" | "is_real" | "is_double" => h("\\HH\\float"),
        "is_string" => h("\\HH\\string"),
        "is_null" => h("\\HH\\void"),
        "HH\\is_keyset" => h("\\HH\\keyset"),
        "HH\\is_dict" => h("\\HH\\dict"),
        "HH\\is_vec" => h("\\HH\\vec"),
        _ => None,
    }
}

fn emit_eval<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        emit_pos(pos),
        instr::eval(),
    ]))
}

fn has_reified_types<'a, 'arena>(env: &Env<'a, 'arena>) -> bool {
    for param in env.scope.get_tparams() {
        match param.reified {
            oxidized::ast::ReifyKind::Reified => {
                return true;
            }
            _ => {}
        }
    }
    false
}

fn emit_lvar<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    _pos: &Pos,
    e: &'a aast_defs::Lid,
) -> Result<InstrSeq<'arena>> {
    use aast_defs::Lid;
    let Lid(pos, _) = e;
    Ok(InstrSeq::gather(vec![
        emit_pos(pos),
        emit_local(emitter, env, BareThisOp::Notice, e)?,
    ]))
}

fn emit_lit<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expression: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let tv = constant_folder::expr_to_typed_value(emitter, &env.scope, expression)
        .map_err(|_| Error::unrecoverable("expr_to_typed_value failed"))?;
    Ok(emit_pos_then(
        pos,
        emit_adata::typed_value_into_instr(emitter, tv)?,
    ))
}

fn emit_is_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    is_expr: &'a (ast::Expr, aast::Hint),
) -> Result<InstrSeq<'arena>> {
    let (e, h) = is_expr;
    let is = emit_is(emitter, env, pos, h)?;
    Ok(InstrSeq::gather(vec![emit_expr(emitter, env, e)?, is]))
}

fn emit_array_get_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    e: &'a (ast::Expr, Option<ast::Expr>),
) -> Result<InstrSeq<'arena>> {
    let (base_expr, opt_elem_expr) = e;
    Ok(emit_array_get(
        emitter,
        env,
        pos,
        None,
        QueryMOp::CGet,
        base_expr,
        opt_elem_expr.as_ref(),
        false,
        false,
    )?
    .0)
}

fn emit_pair<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    e: &'a (Option<(ast::Targ, ast::Targ)>, ast::Expr, ast::Expr),
    expression: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let (_, e1, e2) = e.to_owned();
    let fields = mk_afvalues(&[e1, e2]);
    emit_named_collection(emitter, env, pos, expression, &fields, CollectionType::Pair)
}

fn emit_keyval_collection_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    e: &'a (
        (Pos, ast::KvcKind),
        Option<(ast::Targ, ast::Targ)>,
        Vec<ast::Field>,
    ),
    expression: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let (kind, _, fields) = e;
    let fields = mk_afkvalues(
        fields
            .iter()
            .map(|ast::Field(e1, e2)| (e1.clone(), e2.clone())),
    );
    let collection_typ = match kind.1 {
        aast_defs::KvcKind::Dict => {
            let instr = emit_collection(emitter, env, expression, &fields, None)?;
            return Ok(emit_pos_then(&kind.0, instr));
        }
        aast_defs::KvcKind::Map => CollectionType::Map,
        aast_defs::KvcKind::ImmMap => CollectionType::ImmMap,
    };
    emit_named_collection(emitter, env, pos, expression, &fields, collection_typ)
}

fn emit_val_collection<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    e: &'a ((Pos, ast::VcKind), Option<ast::Targ>, Vec<ast::Expr>),
    expression: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let (kind, _, es) = e;
    let fields = mk_afvalues(es);
    let collection_typ = match kind.1 {
        aast_defs::VcKind::Vec | aast_defs::VcKind::Keyset => {
            let instr = emit_collection(emitter, env, expression, &fields, None)?;
            return Ok(emit_pos_then(&kind.0, instr));
        }
        aast_defs::VcKind::Vector => CollectionType::Vector,
        aast_defs::VcKind::ImmVector => CollectionType::ImmVector,
        aast_defs::VcKind::Set => CollectionType::Set,
        aast_defs::VcKind::ImmSet => CollectionType::ImmSet,
    };
    emit_named_collection(emitter, env, pos, expression, &fields, collection_typ)
}

fn emit_class_get_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    _pos: &Pos,
    e: &'a (ast::ClassId, ast::ClassGetExpr, ast::PropOrMethod),
) -> Result<InstrSeq<'arena>> {
    // class gets without a readonly expression must be mutable
    emit_class_get(
        emitter,
        env,
        QueryMOp::CGet,
        &e.0,
        &e.1,
        ReadonlyOp::Mutable,
    )
}

fn emit_obj_get_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    e: &'a (
        ast::Expr,
        ast::Expr,
        aast_defs::OgNullFlavor,
        aast_defs::PropOrMethod,
    ),
) -> Result<InstrSeq<'arena>> {
    Ok(emit_obj_get(
        emitter,
        env,
        pos,
        QueryMOp::CGet,
        &e.0,
        &e.1,
        &e.2,
        false,
        false,
    )?
    .0)
}

fn emit_label<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    label: &'a (Option<aast_defs::ClassName>, String),
) -> Result<InstrSeq<'arena>> {
    if emitter.options().hhbc.enable_native_enum_class_labels {
        Ok(InstrSeq::gather(vec![
            emit_pos(pos),
            instr::enum_class_label(Str::new_str(env.arena, &label.1)),
        ]))
    } else {
        use ast::Expr_;

        // emitting E#A as __SystemLib\create_opaque_value(OpaqueValue::EnumClassLabel, "A")
        let create_opaque_value = "__SystemLib\\create_opaque_value".to_string();
        let create_opaque_value = ast_defs::Id(pos.clone(), create_opaque_value);
        let create_opaque_value = Expr_::Id(Box::new(create_opaque_value));
        let create_opaque_value = ast::Expr((), pos.clone(), create_opaque_value);
        // OpaqueValue::EnumClassLabel = 0 defined in
        // hphp/runtime/ext/hh/ext_hh.php
        let enum_class_label_index = Expr_::Int("0".to_string());
        let enum_class_label_index = ast::Expr((), pos.clone(), enum_class_label_index);
        let label_string = label.1.to_string();
        let label = Expr_::String(bstr::BString::from(label_string));
        let label = ast::Expr((), pos.clone(), label);
        let call_expr = ast::CallExpr {
            func: create_opaque_value,
            targs: vec![],
            args: vec![
                (ParamKind::Pnormal, enum_class_label_index),
                (ParamKind::Pnormal, label),
            ],
            unpacked_arg: None,
        };
        emit_call_expr(emitter, env, pos, None, false, &call_expr)
    }
}

fn emit_call_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    async_eager_label: Option<Label>,
    readonly_return: bool,
    call_expr: &ast::CallExpr,
) -> Result<InstrSeq<'arena>> {
    use ast::Expr;
    use ast::Expr_;
    let ast::CallExpr {
        func,
        targs,
        args,
        unpacked_arg,
    } = call_expr;
    match (&func.2, &args[..], unpacked_arg) {
        (Expr_::Id(id), _, None) if id.1 == pseudo_functions::ISSET => {
            emit_call_isset_exprs(e, env, pos, args)
        }
        (Expr_::Id(id), args, None)
            if (id.1 == fb::IDX || id.1 == fb::IDXREADONLY)
                && !e.options().function_is_renamable(&id.1)
                && (args.len() == 2 || args.len() == 3) =>
        {
            emit_idx(e, env, pos, args)
        }
        (Expr_::Id(id), [(pk, arg1)], None) if id.1 == emitter_special_functions::EVAL => {
            error::ensure_normal_paramkind(pk)?;
            emit_eval(e, env, pos, arg1)
        }
        (Expr_::Id(id), [(pk, arg1)], None)
            if id.1 == emitter_special_functions::SET_FRAME_METADATA =>
        {
            error::ensure_normal_paramkind(pk)?;
            Ok(InstrSeq::gather(vec![
                emit_expr(e, env, arg1)?,
                emit_pos(pos),
                instr::pop_l(e.named_local("$86metadata".into())),
                instr::null(),
            ]))
        }
        (Expr_::Id(id), [(pk, arg1)], None)
            if id.1 == emitter_special_functions::SET_PRODUCT_ATTRIBUTION_ID
                || id.1 == emitter_special_functions::SET_PRODUCT_ATTRIBUTION_ID_DEFERRED =>
        {
            error::ensure_normal_paramkind(pk)?;
            Ok(InstrSeq::gather(vec![
                emit_expr(e, env, arg1)?,
                emit_pos(pos),
                instr::pop_l(e.named_local("$86productAttributionData".into())),
                instr::null(),
            ]))
        }
        (Expr_::Id(id), [], None) if id.1 == pseudo_functions::EXIT => {
            let exit = emit_exit(e, env, None)?;
            Ok(emit_pos_then(pos, exit))
        }
        (Expr_::Id(id), [(pk, arg1)], None) if id.1 == pseudo_functions::EXIT => {
            error::ensure_normal_paramkind(pk)?;
            let exit = emit_exit(e, env, Some(arg1))?;
            Ok(emit_pos_then(pos, exit))
        }
        (Expr_::Id(id), [], _)
            if id.1 == emitter_special_functions::SYSTEMLIB_REIFIED_GENERICS
                && e.systemlib()
                && has_reified_types(env) =>
        {
            // Rewrite __systemlib_reified_generics() to $0ReifiedGenerics,
            // but only in systemlib functions that take a reified generic.
            let lvar = Expr::new(
                (),
                pos.clone(),
                Expr_::Lvar(Box::new(ast::Lid(
                    pos.clone(),
                    local_id::make_unscoped(string_utils::reified::GENERICS_LOCAL_NAME),
                ))),
            );
            emit_expr(e, env, &lvar)
        }
        (Expr_::Id(id), [arg], None)
            if id.1 == "\\__hhvm_intrinsics\\isTypeStructShallow" && targs.len() == 1 =>
        {
            let is = emit_is_shallow(e, env, pos, &targs[0].1)?;
            Ok(InstrSeq::gather(vec![
                emit_expr(e, env, error::expect_normal_paramkind(arg)?)?,
                is,
            ]))
        }
        (_, _, _) => {
            let instrs = emit_call(
                e,
                env,
                pos,
                func,
                targs,
                args,
                unpacked_arg.as_ref(),
                async_eager_label,
                readonly_return,
            )?;
            Ok(emit_pos_then(pos, instrs))
        }
    }
}

pub fn emit_reified_generic_instrs<'arena>(
    e: &Emitter<'arena, '_>,
    pos: &Pos,
    index: ReifiedTparam,
) -> InstrSeq<'arena> {
    fn q<'arena>(i: usize) -> InstrSeq<'arena> {
        instr::query_m(
            0,
            QueryMOp::CGet,
            MemberKey::EI(i.try_into().unwrap(), ReadonlyOp::Any),
        )
    }
    let seq = match index {
        ReifiedTparam::Fun(i) => InstrSeq::gather(vec![
            instr::base_l(
                e.named_local(string_utils::reified::GENERICS_LOCAL_NAME.into()),
                MOpMode::Warn,
                ReadonlyOp::Any,
            ),
            q(i),
        ]),
        ReifiedTparam::Class(i) => InstrSeq::gather(vec![
            instr::check_this(),
            instr::base_h(),
            instr::dim_warn_pt(
                hhbc::PropName::from_raw_string(e.alloc, string_utils::reified::PROP_NAME),
                ReadonlyOp::Any,
            ),
            q(i),
        ]),
    };
    emit_pos_then(pos, seq)
}

fn emit_reified_type<'a, 'arena>(
    e: &Emitter<'arena, '_>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    name: &str,
) -> Result<InstrSeq<'arena>> {
    let is_in_lambda = env.scope.is_in_lambda();
    let cget_instr = |i| {
        instr::c_get_l(
            e.named_local(
                string_utils::reified::reified_generic_captured_name(i)
                    .as_str()
                    .into(),
            ),
        )
    };
    match ClassExpr::get_reified_tparam(&env.scope, name) {
        Some((i, is_soft)) => {
            if is_soft {
                return Err(Error::fatal_parse(
                    pos,
                    format!(
                        "{} is annotated to be a soft reified generic, it cannot be used until the __Soft annotation is removed",
                        name
                    ),
                ));
            }
            if is_in_lambda {
                Ok(cget_instr(i))
            } else {
                Ok(emit_reified_generic_instrs(e, pos, i))
            }
        }
        None => Err(Error::fatal_runtime(pos, "Invalid reified param")),
    }
}

fn emit_known_class_id<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl>,
    id: &ast_defs::Id,
) -> InstrSeq<'arena> {
    let cid = hhbc::ClassName::from_ast_name_and_mangle(alloc, &id.1);
    e.add_class_ref(cid);
    instr::resolve_class(cid)
}

fn emit_load_class_ref<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    cexpr: ClassExpr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let instrs = match cexpr {
        ClassExpr::Special(clsref) => match clsref {
            SpecialClsRef::SelfCls => instr::self_cls(),
            SpecialClsRef::LateBoundCls => instr::late_bound_cls(),
            SpecialClsRef::ParentCls => instr::parent_cls(),
            _ => unreachable!(),
        },
        ClassExpr::Id(id) => emit_known_class_id(alloc, e, &id),
        ClassExpr::Expr(expr) => InstrSeq::gather(vec![
            emit_pos(pos),
            emit_expr(e, env, &expr)?,
            instr::class_get_c(ClassGetCMode::Normal),
        ]),
        ClassExpr::Reified(ast::Id(p, name)) => InstrSeq::gather(vec![
            emit_pos(pos),
            get_reified_var_cexpr(e, env, &p, &name)?,
            instr::class_get_c(ClassGetCMode::Normal),
        ]),
    };
    Ok(emit_pos_then(pos, instrs))
}

enum NewObjOpInfo<'a, 'arena> {
    /// true => top of stack contains type structure
    /// false => top of stack contains class
    NewObj(bool),
    NewObjD(&'a hhbc::ClassName<'arena>, Option<&'a Vec<ast::Targ>>),
    NewObjS(hhbc::SpecialClsRef),
}

fn emit_new<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    (cid, targs, args, uarg, _): &(
        ast::ClassId,
        Vec<ast::Targ>,
        Vec<ast::Expr>,
        Option<ast::Expr>,
        (),
    ),
    is_reflection_class_builtin: bool,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let resolve_self = match &cid.2.as_ciexpr() {
        Some(ci_expr) => match ci_expr.as_id() {
            Some(ast_defs::Id(_, n)) if string_utils::is_self(n) => env
                .scope
                .get_class_tparams()
                .iter()
                .all(|tp| tp.reified.is_erased()),
            Some(ast_defs::Id(_, n)) if string_utils::is_parent(n) => {
                env.scope
                    .get_class()
                    .map_or(true, |cls| match cls.get_extends() {
                        [h, ..] => {
                            h.1.as_happly()
                                .map_or(true, |(_, l)| !has_non_tparam_generics(env, l))
                        }
                        _ => true,
                    })
            }
            _ => true,
        },
        _ => true,
    };
    let cexpr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, resolve_self, cid);
    if is_reflection_class_builtin {
        scope::with_unnamed_locals(e, |e| {
            let instr_args = emit_exprs(e, env, args)?;
            let instr_uargs = match uarg {
                None => instr::empty(),
                Some(uarg) => emit_expr(e, env, uarg)?,
            };
            Ok((
                instr::empty(),
                InstrSeq::gather(vec![instr_args, instr_uargs]),
                instr::empty(),
            ))
        })
    } else {
        let newobj_instrs = match cexpr {
            ClassExpr::Id(ast_defs::Id(_, cname)) => {
                let id = hhbc::ClassName::<'arena>::from_ast_name_and_mangle(alloc, &cname);
                e.add_class_ref(id);
                let targs = if has_non_tparam_generics_targs(env, targs) {
                    Some(targs)
                } else {
                    None
                };
                emit_new_obj_reified_instrs(e, env, pos, NewObjOpInfo::NewObjD(&id, targs))?
            }
            ClassExpr::Special(cls_ref) => {
                emit_new_obj_reified_instrs(e, env, pos, NewObjOpInfo::NewObjS(cls_ref))?
            }
            ClassExpr::Reified(ast::Id(p, name)) => {
                if !targs.is_empty() {
                    return Err(Error::fatal_parse(
                        pos,
                        "Cannot have higher kinded reified generics",
                    ));
                }
                InstrSeq::gather(vec![
                    emit_reified_type(e, env, &p, &name)?,
                    emit_new_obj_reified_instrs(e, env, pos, NewObjOpInfo::NewObj(true))?,
                ])
            }
            ClassExpr::Expr(_) => InstrSeq::gather(vec![
                emit_load_class_ref(e, env, pos, cexpr)?,
                emit_new_obj_reified_instrs(e, env, pos, NewObjOpInfo::NewObj(false))?,
            ]),
        };
        scope::with_unnamed_locals(e, |e| {
            let alloc = e.alloc;
            let instr_args = emit_exprs(e, env, args)?;
            let instr_uargs = match uarg {
                None => instr::empty(),
                Some(uarg) => emit_expr(e, env, uarg)?,
            };
            Ok((
                instr::empty(),
                InstrSeq::gather(vec![
                    newobj_instrs,
                    instr::dup(),
                    instr::null_uninit(),
                    instr_args,
                    instr_uargs,
                    emit_pos(pos),
                    instr::f_call_ctor(get_fcall_args_no_inout(
                        alloc,
                        args,
                        uarg.as_ref(),
                        None,
                        env.call_context.clone(),
                        true,
                        true,  // we do not need to enforce readonly return for constructors
                        false, // we do not need to enforce readonly this for constructors
                    )),
                    instr::pop_c(),
                    instr::lock_obj(),
                ]),
                instr::empty(),
            ))
        })
    }
}

fn emit_new_obj_reified_instrs<'a, 'b, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    op: NewObjOpInfo<'b, 'arena>,
) -> Result<InstrSeq<'arena>> {
    use string_utils::reified::INIT_METH_NAME;

    let call_reified_init = |obj, ts| -> InstrSeq<'_> {
        InstrSeq::gather(vec![
            obj,
            instr::null_uninit(),
            ts,
            instr::f_call_obj_method_d(
                FCallArgs::new(FCallArgsFlags::default(), 1, 1, vec![], vec![], None, None),
                hhbc::MethodName::from_raw_string(env.arena, INIT_METH_NAME),
            ),
            instr::pop_c(),
        ])
    };

    scope::with_unnamed_locals(e, |e| {
        let class_local = e.local_gen_mut().get_unnamed();
        let ts_local = e.local_gen_mut().get_unnamed();
        let obj_local = e.local_gen_mut().get_unnamed();

        let no_reified_generics_passed_in_label = e.label_gen_mut().next_regular();
        let try_parent_has_reified_generics_label = e.label_gen_mut().next_regular();
        let end_label = e.label_gen_mut().next_regular();

        let try_parent_has_reified_generics = InstrSeq::gather(vec![
            instr::label(try_parent_has_reified_generics_label),
            instr::c_get_l(class_local),
            instr::has_reified_parent(),
            instr::jmp_z(end_label),
            call_reified_init(
                instr::c_get_l(obj_local),
                InstrSeq::gather(vec![
                    instr::new_col(CollectionType::Vector),
                    instr::cast_vec(),
                ]),
            ),
            instr::jmp(end_label),
        ]);

        let is_ts_empty = InstrSeq::gather(vec![
            instr::null_uninit(),
            instr::null_uninit(),
            instr::c_get_l(ts_local),
            instr::f_call_func_d(
                FCallArgs::new(FCallArgsFlags::default(), 1, 1, vec![], vec![], None, None),
                hhbc::FunctionName::from_raw_string(env.arena, "count"),
            ),
            instr::int(0),
            instr::eq(),
        ]);

        let no_reified_generics_passed_in = InstrSeq::gather(vec![
            instr::label(no_reified_generics_passed_in_label),
            instr::c_get_l(class_local),
            instr::class_has_reified_generics(),
            instr::jmp_z(try_parent_has_reified_generics_label),
            instr::c_get_l(class_local),
            instr::check_cls_rg_soft(),
            instr::jmp(end_label),
        ]);

        let reified_generics_passed_in = InstrSeq::gather(vec![
            instr::c_get_l(class_local),
            instr::class_has_reified_generics(),
            instr::jmp_z(try_parent_has_reified_generics_label),
            call_reified_init(instr::c_get_l(obj_local), instr::c_get_l(ts_local)),
            instr::jmp(end_label),
        ]);

        let instrs = match op {
            NewObjOpInfo::NewObj(false) => InstrSeq::gather(vec![
                instr::set_l(class_local),
                instr::new_obj(),
                instr::pop_l(obj_local),
                no_reified_generics_passed_in,
                try_parent_has_reified_generics,
            ]),
            NewObjOpInfo::NewObj(true) => InstrSeq::gather(vec![
                instr::class_get_ts(),
                instr::pop_l(ts_local),
                instr::set_l(class_local),
                instr::new_obj(),
                instr::pop_l(obj_local),
                is_ts_empty,
                instr::jmp_nz(no_reified_generics_passed_in_label),
                reified_generics_passed_in,
                no_reified_generics_passed_in,
                try_parent_has_reified_generics,
            ]),
            NewObjOpInfo::NewObjD(id, targs) => {
                let ts = match targs {
                    Some(targs) => emit_reified_targs(e, env, pos, targs.iter().map(|t| &t.1))?,
                    None => instr::empty(),
                };
                let store_cls_and_obj = InstrSeq::gather(vec![
                    instr::resolve_class(*id),
                    instr::pop_l(class_local),
                    instr::new_obj_d(*id),
                    instr::pop_l(obj_local),
                ]);
                if ts.is_empty() {
                    InstrSeq::gather(vec![
                        store_cls_and_obj,
                        no_reified_generics_passed_in,
                        try_parent_has_reified_generics,
                    ])
                } else {
                    InstrSeq::gather(vec![
                        ts,
                        instr::pop_l(ts_local),
                        store_cls_and_obj,
                        reified_generics_passed_in,
                        try_parent_has_reified_generics,
                    ])
                }
            }
            NewObjOpInfo::NewObjS(cls_ref) => {
                let get_cls_instr = match cls_ref {
                    SpecialClsRef::SelfCls => instr::self_cls(),
                    SpecialClsRef::LateBoundCls => instr::late_bound_cls(),
                    SpecialClsRef::ParentCls => instr::parent_cls(),
                    _ => {
                        return Err(Error::unrecoverable(
                            "Internal error: This case should never occur",
                        ));
                    }
                };
                let store_cls_and_obj = InstrSeq::gather(vec![
                    get_cls_instr,
                    instr::pop_l(class_local),
                    instr::new_obj_s(cls_ref),
                    instr::pop_l(obj_local),
                ]);
                let store_ts_if_prop_set = InstrSeq::gather(vec![
                    instr::c_get_l(class_local),
                    instr::class_has_reified_generics(),
                    instr::jmp_z(no_reified_generics_passed_in_label),
                    instr::c_get_l(class_local),
                    instr::get_cls_rg_prop(),
                    instr::set_l(ts_local),
                    instr::is_type_c(IsTypeOp::Null),
                    instr::jmp_nz(no_reified_generics_passed_in_label),
                ]);
                InstrSeq::gather(vec![
                    store_cls_and_obj,
                    store_ts_if_prop_set,
                    is_ts_empty,
                    instr::jmp_nz(no_reified_generics_passed_in_label),
                    reified_generics_passed_in,
                    no_reified_generics_passed_in,
                    try_parent_has_reified_generics,
                ])
            }
        };

        Ok((
            instr::empty(),
            InstrSeq::gather(vec![
                emit_pos(pos),
                instrs,
                instr::label(end_label),
                instr::c_get_l(obj_local),
            ]),
            instr::empty(),
        ))
    })
}

fn emit_obj_get<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    query_op: QueryMOp,
    expr: &ast::Expr,
    prop: &ast::Expr,
    nullflavor: &ast_defs::OgNullFlavor,
    null_coalesce_assignment: bool,
    readonly_get: bool, // obj_get enclosed in readonly expression
) -> Result<(InstrSeq<'arena>, Option<StackIndex>)> {
    let readonly_op = if readonly_get {
        ReadonlyOp::Any
    } else {
        ReadonlyOp::Mutable
    };

    if let Some(ast::Lid(pos, id)) = expr.2.as_lvar() {
        if local_id::get_name(id) == special_idents::THIS
            && nullflavor.eq(&ast_defs::OgNullFlavor::OGNullsafe)
        {
            return Err(Error::fatal_parse(pos, "?-> is not allowed with $this"));
        }
    }
    if let Some(ast_defs::Id(_, s)) = prop.2.as_id() {
        if string_utils::is_xhp(s) {
            return Ok((emit_xhp_obj_get(e, env, pos, expr, s, nullflavor)?, None));
        }
    }
    let mode = if null_coalesce_assignment {
        MOpMode::Warn
    } else {
        get_querym_op_mode(&query_op)
    };
    let prop_stack_size = emit_prop_expr(
        e,
        env,
        nullflavor,
        0,
        prop,
        null_coalesce_assignment,
        ReadonlyOp::Any,
    )?
    .2;
    let (
        base_expr_instrs_begin,
        base_expr_instrs_end,
        base_setup_instrs,
        base_stack_size,
        cls_stack_size,
    ) = emit_base(
        e,
        env,
        expr,
        mode,
        true,
        BareThisOp::Notice,
        null_coalesce_assignment,
        prop_stack_size,
        0,
        readonly_op,
    )?;
    let (mk, prop_instrs, _) = emit_prop_expr(
        e,
        env,
        nullflavor,
        cls_stack_size,
        prop,
        null_coalesce_assignment,
        readonly_op,
    )?;
    let total_stack_size = (prop_stack_size + base_stack_size + cls_stack_size) as usize;
    let num_params = if null_coalesce_assignment {
        0
    } else {
        total_stack_size
    };
    let final_instr = instr::query_m(num_params as u32, query_op, mk);
    // Don't pop elems/props from the stack during the lookup for null
    // coalesce assignment in case we do a write later.
    let querym_n_unpopped = if null_coalesce_assignment {
        Some(total_stack_size as u32)
    } else {
        None
    };
    let instr = InstrSeq::gather(vec![
        base_expr_instrs_begin,
        prop_instrs,
        base_expr_instrs_end,
        emit_pos(pos),
        base_setup_instrs,
        final_instr,
    ]);
    Ok((instr, querym_n_unpopped))
}

// Get the member key for a property, and return any instructions and
// the size of the stack in the case that the property cannot be
// placed inline in the instruction.
fn emit_prop_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    nullflavor: &ast_defs::OgNullFlavor,
    stack_index: StackIndex,
    prop: &ast::Expr,
    null_coalesce_assignment: bool,
    readonly_op: ReadonlyOp,
) -> Result<(MemberKey<'arena>, InstrSeq<'arena>, StackIndex)> {
    let alloc = env.arena;

    let mk = match &prop.2 {
        ast::Expr_::Id(id) => {
            let ast_defs::Id(pos, name) = &**id;
            if name.starts_with('$') {
                MemberKey::PL(get_local(e, env, pos, name)?, readonly_op)
            } else {
                // Special case for known property name
                let pid = hhbc::PropName::<'arena>::from_ast_name(
                    alloc,
                    string_utils::strip_global_ns(name),
                );
                match nullflavor {
                    ast_defs::OgNullFlavor::OGNullthrows => MemberKey::PT(pid, readonly_op),
                    ast_defs::OgNullFlavor::OGNullsafe => MemberKey::QT(pid, readonly_op),
                }
            }
        }
        // Special case for known property name
        ast::Expr_::String(name) => {
            let pid: hhbc::PropName<'arena> = hhbc::PropName::<'arena>::from_ast_name(
                alloc,
                string_utils::strip_global_ns(
                    // FIXME: This is not safe--string literals are binary strings.
                    // There's no guarantee that they're valid UTF-8.
                    unsafe { std::str::from_utf8_unchecked(name.as_slice()) },
                ),
            );
            match nullflavor {
                ast_defs::OgNullFlavor::OGNullthrows => MemberKey::PT(pid, readonly_op),
                ast_defs::OgNullFlavor::OGNullsafe => MemberKey::QT(pid, readonly_op),
            }
        }
        ast::Expr_::Lvar(lid) if !(is_local_this(env, &lid.1)) => MemberKey::PL(
            get_local(e, env, &lid.0, local_id::get_name(&lid.1))?,
            readonly_op,
        ),
        _ => {
            // General case
            MemberKey::PC(stack_index, readonly_op)
        }
    };
    // For nullsafe access, insist that property is known
    Ok(match mk {
        MemberKey::PL(_, _) | MemberKey::PC(_, _)
            if nullflavor.eq(&ast_defs::OgNullFlavor::OGNullsafe) =>
        {
            return Err(Error::fatal_parse(
                &prop.1,
                "?-> can only be used with scalar property names",
            ));
        }
        MemberKey::PC(_, _) => (mk, emit_expr(e, env, prop)?, 1),
        MemberKey::PL(local, readonly_op) if null_coalesce_assignment => (
            MemberKey::PC(stack_index, readonly_op),
            instr::c_get_l(local),
            1,
        ),
        _ => (mk, instr::empty(), 0),
    })
}

fn emit_xhp_obj_get<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
    s: &str,
    nullflavor: &ast_defs::OgNullFlavor,
) -> Result<InstrSeq<'arena>> {
    use ast::Expr;
    use ast::Expr_;
    let f = Expr(
        (),
        pos.clone(),
        Expr_::mk_obj_get(
            expr.clone(),
            Expr(
                (),
                pos.clone(),
                Expr_::mk_id(ast_defs::Id(pos.clone(), "getAttribute".into())),
            ),
            nullflavor.clone(),
            ast::PropOrMethod::IsMethod,
        ),
    );
    let args = vec![(
        ParamKind::Pnormal,
        Expr(
            (),
            pos.clone(),
            Expr_::mk_string(string_utils::clean(s).into()),
        ),
    )];
    emit_call(e, env, pos, &f, &[], &args[..], None, None, false)
}

fn emit_array_get<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    mode: Option<MOpMode>,
    query_op: QueryMOp,
    base: &ast::Expr,
    elem: Option<&ast::Expr>,
    no_final: bool,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq<'arena>, Option<StackIndex>)> {
    let result = emit_array_get_(
        e,
        env,
        outer_pos,
        mode,
        query_op,
        base,
        elem,
        no_final,
        null_coalesce_assignment,
        None,
    )?;
    match result {
        (ArrayGetInstr::Regular(i), querym_n_unpopped) => Ok((i, querym_n_unpopped)),
        (ArrayGetInstr::Inout { .. }, _) => Err(Error::unrecoverable("unexpected inout")),
    }
}

fn emit_array_get_<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    mode: Option<MOpMode>,
    query_op: QueryMOp,
    base_expr: &ast::Expr,
    elem: Option<&ast::Expr>,
    no_final: bool,
    null_coalesce_assignment: bool,
    inout_param_info: Option<(usize, &inout_locals::AliasInfoMap<'_>)>,
) -> Result<(ArrayGetInstr<'arena>, Option<StackIndex>)> {
    use ast::Expr;

    match (base_expr, elem) {
        (Expr(_, pos, _), None) if !env.flags.contains(env::Flags::ALLOWS_ARRAY_APPEND) => {
            Err(Error::fatal_runtime(pos, "Can't use [] for reading"))
        }
        _ => {
            let local_temp_kind = get_local_temp_kind(env, false, inout_param_info, elem);
            let mode = if null_coalesce_assignment {
                MOpMode::Warn
            } else {
                mode.unwrap_or_else(|| get_querym_op_mode(&query_op))
            };
            let (elem_instrs, elem_stack_size) =
                emit_elem(e, env, elem, local_temp_kind, null_coalesce_assignment)?;
            let base_result = emit_base_(
                e,
                env,
                base_expr,
                mode,
                false,
                match query_op {
                    QueryMOp::Isset => BareThisOp::NoNotice,
                    _ => BareThisOp::Notice,
                },
                null_coalesce_assignment,
                elem_stack_size,
                0,
                inout_param_info,
                ReadonlyOp::Any, // array get on reading has no restrictions
            )?;
            let cls_stack_size = match &base_result {
                ArrayGetBase::Regular(base) => base.cls_stack_size,
                ArrayGetBase::Inout { load, .. } => load.cls_stack_size,
            };
            let (memberkey, warninstr) =
                get_elem_member_key(e, env, cls_stack_size, elem, null_coalesce_assignment)?;
            let mut querym_n_unpopped = None;
            let mut make_final =
                |total_stack_size: StackIndex, memberkey: MemberKey<'arena>| -> InstrSeq<'_> {
                    if no_final {
                        instr::empty()
                    } else if null_coalesce_assignment {
                        querym_n_unpopped = Some(total_stack_size);
                        instr::query_m(0, query_op, memberkey)
                    } else {
                        instr::query_m(total_stack_size, query_op, memberkey)
                    }
                };
            let instr = match (base_result, local_temp_kind) {
                (ArrayGetBase::Regular(base), None) =>
                // neither base nor expression needs to store anything
                {
                    ArrayGetInstr::Regular(InstrSeq::gather(vec![
                        warninstr,
                        base.base_instrs,
                        elem_instrs,
                        base.cls_instrs,
                        emit_pos(outer_pos),
                        base.setup_instrs,
                        make_final(
                            base.base_stack_size + base.cls_stack_size + elem_stack_size,
                            memberkey,
                        ),
                    ]))
                }
                (ArrayGetBase::Regular(base), Some(local_kind)) => {
                    // base does not need temp locals but index expression does
                    let local = e.local_gen_mut().get_unnamed();
                    let load = vec![
                        // load base and indexer, value of indexer will be saved in local
                        (
                            InstrSeq::gather(vec![base.base_instrs.clone(), elem_instrs]),
                            Some((local, local_kind)),
                        ),
                        // finish loading the value
                        (
                            InstrSeq::gather(vec![
                                warninstr,
                                base.base_instrs,
                                emit_pos(outer_pos),
                                base.setup_instrs,
                                make_final(
                                    base.base_stack_size + base.cls_stack_size + elem_stack_size,
                                    memberkey,
                                ),
                            ]),
                            None,
                        ),
                    ];
                    let store = InstrSeq::gather(vec![
                        emit_store_for_simple_base(
                            e,
                            env,
                            outer_pos,
                            elem_stack_size,
                            base_expr,
                            local,
                            false,
                            ReadonlyOp::Any,
                        )?,
                        instr::pop_c(),
                    ]);
                    ArrayGetInstr::Inout { load, store }
                }
                (
                    ArrayGetBase::Inout {
                        load:
                            ArrayGetBaseData {
                                mut base_instrs,
                                cls_instrs,
                                setup_instrs,
                                base_stack_size,
                                cls_stack_size,
                            },
                        store,
                    },
                    None,
                ) => {
                    // base needs temp locals, indexer - does not,
                    // simply concat two instruction sequences
                    base_instrs.push((
                        InstrSeq::gather(vec![
                            warninstr,
                            elem_instrs,
                            cls_instrs,
                            emit_pos(outer_pos),
                            setup_instrs,
                            make_final(
                                base_stack_size + cls_stack_size + elem_stack_size,
                                memberkey.clone(),
                            ),
                        ]),
                        None,
                    ));
                    let store =
                        InstrSeq::gather(vec![store, instr::set_m(0, memberkey), instr::pop_c()]);
                    ArrayGetInstr::Inout {
                        load: base_instrs,
                        store,
                    }
                }
                (
                    ArrayGetBase::Inout {
                        load:
                            ArrayGetBaseData {
                                mut base_instrs,
                                cls_instrs,
                                setup_instrs,
                                base_stack_size,
                                cls_stack_size,
                            },
                        store,
                    },
                    Some(local_kind),
                ) => {
                    // both base and index need temp locals,
                    // create local for index value
                    let local = e.local_gen_mut().get_unnamed();
                    base_instrs.push((elem_instrs, Some((local, local_kind))));
                    base_instrs.push((
                        InstrSeq::gather(vec![
                            warninstr,
                            cls_instrs,
                            emit_pos(outer_pos),
                            setup_instrs,
                            make_final(
                                base_stack_size + cls_stack_size + elem_stack_size,
                                memberkey,
                            ),
                        ]),
                        None,
                    ));
                    let store = InstrSeq::gather(vec![
                        store,
                        instr::set_m(0, MemberKey::EL(local, ReadonlyOp::Any)),
                        instr::pop_c(),
                    ]);
                    ArrayGetInstr::Inout {
                        load: base_instrs,
                        store,
                    }
                }
            };
            Ok((instr, querym_n_unpopped))
        }
    }
}

fn is_special_class_constant_accessed_with_class_id(
    e: &Emitter<'_, '_>,
    env: &Env<'_, '_>,
    cname: &ast::ClassId,
    id: &str,
) -> bool {
    if !string_utils::is_class(id) {
        return false;
    }
    let expr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, false, cname);
    matches!(expr, ClassExpr::Id(_))
}

fn emit_elem<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    elem: Option<&ast::Expr>,
    local_temp_kind: Option<StoredValueKind>,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq<'arena>, StackIndex)> {
    Ok(match elem {
        None => (instr::empty(), 0),
        Some(expr) if expr.2.is_int() || expr.2.is_string() => (instr::empty(), 0),
        Some(expr) => match &expr.2 {
            ast::Expr_::Lvar(x) if !is_local_this(env, &x.1) => {
                if local_temp_kind.is_some() {
                    (
                        instr::c_get_quiet_l(get_local(e, env, &x.0, local_id::get_name(&x.1))?),
                        0,
                    )
                } else if null_coalesce_assignment {
                    (
                        instr::c_get_l(get_local(e, env, &x.0, local_id::get_name(&x.1))?),
                        1,
                    )
                } else {
                    (instr::empty(), 0)
                }
            }
            ast::Expr_::ClassConst(x)
                if is_special_class_constant_accessed_with_class_id(e, env, &(x.0), &(x.1).1) =>
            {
                (instr::empty(), 0)
            }
            _ => (emit_expr(e, env, expr)?, 1),
        },
    })
}

fn get_elem_member_key<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    stack_index: StackIndex,
    elem: Option<&ast::Expr>,
    null_coalesce_assignment: bool,
) -> Result<(MemberKey<'arena>, InstrSeq<'arena>)> {
    use ast::ClassId_ as CI_;
    use ast::Expr;
    use ast::Expr_;
    let alloc = env.arena;
    match elem {
        // ELement missing (so it's array append)
        None => Ok((MemberKey::W, instr::empty())),
        Some(elem_expr) => match &elem_expr.2 {
            // Special case for local
            Expr_::Lvar(x) if !is_local_this(env, &x.1) => Ok((
                {
                    if null_coalesce_assignment {
                        MemberKey::EC(stack_index, ReadonlyOp::Any)
                    } else {
                        MemberKey::EL(
                            get_local(e, env, &x.0, local_id::get_name(&x.1))?,
                            ReadonlyOp::Any,
                        )
                    }
                },
                instr::empty(),
            )),
            // Special case for literal integer
            Expr_::Int(s) => match constant_folder::expr_to_typed_value(e, &env.scope, elem_expr) {
                Ok(TypedValue::Int(i)) => Ok((MemberKey::EI(i, ReadonlyOp::Any), instr::empty())),
                _ => Err(Error::unrecoverable(format!(
                    "{} is not a valid integer index",
                    s
                ))),
            },
            // Special case for literal string
            Expr_::String(s) => {
                // FIXME: This is not safe--string literals are binary strings.
                // There's no guarantee that they're valid UTF-8.
                let s = unsafe { std::str::from_utf8_unchecked(s.as_slice()) };
                let s = bumpalo::collections::String::from_str_in(s, alloc).into_bump_str();
                Ok((MemberKey::ET(Str::from(s), ReadonlyOp::Any), instr::empty()))
            }
            // Special case for class name
            Expr_::ClassConst(x)
                if is_special_class_constant_accessed_with_class_id(e, env, &(x.0), &(x.1).1) =>
            {
                let cname = match (&(x.0).2, env.scope.get_class()) {
                    (CI_::CIself, Some(cd)) => string_utils::strip_global_ns(cd.get_name_str()),
                    (CI_::CIexpr(Expr(_, _, Expr_::Id(id))), _) => {
                        string_utils::strip_global_ns(&id.1)
                    }
                    (CI_::CI(id), _) => string_utils::strip_global_ns(&id.1),
                    _ => {
                        return Err(Error::unrecoverable(
                            "Unreachable due to is_special_class_constant_accessed_with_class_id",
                        ));
                    }
                };
                let fq_id = hhbc::ClassName::<'arena>::from_ast_name_and_mangle(alloc, cname)
                    .unsafe_as_str();
                Ok((
                    MemberKey::ET(Str::from(fq_id), ReadonlyOp::Any),
                    instr::raise_class_string_conversion_notice(),
                ))
            }
            _ => {
                // General case
                Ok((MemberKey::EC(stack_index, ReadonlyOp::Any), instr::empty()))
            }
        },
    }
}

fn emit_store_for_simple_base<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    elem_stack_size: StackIndex,
    base: &ast::Expr,
    local: Local,
    is_base: bool,
    readonly_op: ReadonlyOp,
) -> Result<InstrSeq<'arena>> {
    let (base_expr_instrs_begin, base_expr_instrs_end, base_setup_instrs, _, _) = emit_base(
        e,
        env,
        base,
        MOpMode::Define,
        false,
        BareThisOp::Notice,
        false,
        elem_stack_size,
        0,
        readonly_op,
    )?;
    let memberkey = MemberKey::EL(local, ReadonlyOp::Any);
    Ok(InstrSeq::gather(vec![
        base_expr_instrs_begin,
        base_expr_instrs_end,
        emit_pos(pos),
        base_setup_instrs,
        if is_base {
            instr::dim(MOpMode::Define, memberkey)
        } else {
            instr::set_m(0, memberkey)
        },
    ]))
}

fn get_querym_op_mode(query_op: &QueryMOp) -> MOpMode {
    match *query_op {
        QueryMOp::InOut => MOpMode::InOut,
        QueryMOp::CGet => MOpMode::Warn,
        _ => MOpMode::None,
    }
}

fn emit_class_get<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    query_op: QueryMOp,
    cid: &ast::ClassId,
    prop: &ast::ClassGetExpr,
    readonly_op: ReadonlyOp,
) -> Result<InstrSeq<'arena>> {
    let cexpr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, false, cid);
    let (cexpr_seq1, cexpr_seq2) = emit_class_expr(e, env, cexpr, prop)?;
    Ok(InstrSeq::gather(vec![
        cexpr_seq1,
        cexpr_seq2,
        match query_op {
            QueryMOp::CGet => instr::c_get_s(readonly_op),
            QueryMOp::Isset => instr::isset_s(),
            QueryMOp::CGetQuiet => {
                return Err(Error::unrecoverable("emit_class_get: CGetQuiet"));
            }
            QueryMOp::InOut => return Err(Error::unrecoverable("emit_class_get: InOut")),
            _ => panic!("Enum value does not match one of listed variants"),
        },
    ]))
}

fn emit_conditional_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    etest: &ast::Expr,
    etrue: Option<&ast::Expr>,
    efalse: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    Ok(match etrue.as_ref() {
        Some(etrue) => {
            let false_label = e.label_gen_mut().next_regular();
            let end_label = e.label_gen_mut().next_regular();
            let r = emit_jmpz(e, env, etest, false_label)?;
            // only emit false branch if false_label is used
            let false_branch = if r.is_label_used {
                InstrSeq::gather(vec![instr::label(false_label), emit_expr(e, env, efalse)?])
            } else {
                instr::empty()
            };
            // only emit true branch if there is fallthrough from condition
            let true_branch = if r.is_fallthrough {
                InstrSeq::gather(vec![
                    emit_expr(e, env, etrue)?,
                    emit_pos(pos),
                    instr::jmp(end_label),
                ])
            } else {
                instr::empty()
            };
            InstrSeq::gather(vec![
                r.instrs,
                true_branch,
                false_branch,
                // end_label is used to jump out of true branch so they should be emitted together
                if r.is_fallthrough {
                    instr::label(end_label)
                } else {
                    instr::empty()
                },
            ])
        }
        None => {
            let end_label = e.label_gen_mut().next_regular();
            let efalse_instr = emit_expr(e, env, efalse)?;
            let etest_instr = emit_expr(e, env, etest)?;
            InstrSeq::gather(vec![
                etest_instr,
                instr::dup(),
                instr::jmp_nz(end_label),
                instr::pop_c(),
                efalse_instr,
                instr::label(end_label),
            ])
        }
    })
}

fn emit_local<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    notice: BareThisOp,
    lid: &aast_defs::Lid,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let ast::Lid(pos, id) = lid;
    let id_name = local_id::get_name(id);
    if superglobals::is_superglobal(id_name) {
        Ok(InstrSeq::gather(vec![
            instr::string(alloc, string_utils::locals::strip_dollar(id_name)),
            emit_pos(pos),
            instr::c_get_g(),
        ]))
    } else {
        Ok(
            if is_local_this(env, id) && !env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) {
                emit_pos_then(pos, instr::bare_this(notice))
            } else {
                let local = get_local(e, env, pos, id_name)?;
                instr::c_get_l(local)
            },
        )
    }
}

fn emit_class_const<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    cid: &ast::ClassId,
    id: &ast_defs::Pstring,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let cexpr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, true, cid);
    match cexpr {
        ClassExpr::Id(ast_defs::Id(pos, name)) => {
            let cid = hhbc::ClassName::from_ast_name_and_mangle(alloc, &name);
            Ok(if string_utils::is_class(&id.1) {
                emit_pos_then(&pos, instr::lazy_class(cid))
            } else {
                e.add_class_ref(cid.clone());
                // TODO(hrust) enabel `let const_id = hhbc::ConstName::from_ast_name(&id.1);`,
                // `from_ast_name` should be able to accpet Cow<str>
                let const_id =
                    hhbc::ConstName::new(Str::new_str(alloc, string_utils::strip_global_ns(&id.1)));
                emit_pos_then(&pos, instr::cls_cns_d(const_id, cid))
            })
        }
        _ => {
            let load_const = if string_utils::is_class(&id.1) {
                instr::lazy_class_from_class()
            } else {
                // TODO(hrust) enable `let const_id = hhbc::ConstName::from_ast_name(&id.1);`,
                // `from_ast_name` should be able to accpet Cow<str>
                let const_id =
                    hhbc::ConstName::new(Str::new_str(alloc, string_utils::strip_global_ns(&id.1)));
                instr::cls_cns(const_id)
            };
            Ok(InstrSeq::gather(vec![
                emit_load_class_ref(e, env, pos, cexpr)?,
                load_const,
            ]))
        }
    }
}

fn emit_unop<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    (uop, expr): &(ast_defs::Uop, ast::Expr),
) -> Result<InstrSeq<'arena>> {
    use ast_defs::Uop;

    match uop {
        Uop::Utild | Uop::Unot => Ok(InstrSeq::gather(vec![
            emit_expr(e, env, expr)?,
            emit_pos_then(pos, from_unop(uop)?),
        ])),
        Uop::Uplus | Uop::Uminus => Ok(InstrSeq::gather(vec![
            emit_pos(pos),
            instr::int(0),
            emit_expr(e, env, expr)?,
            emit_pos_then(pos, from_unop(uop)?),
        ])),
        Uop::Uincr | Uop::Udecr | Uop::Upincr | Uop::Updecr => emit_lval_op(
            e,
            env,
            pos,
            LValOp::IncDec(unop_to_incdec_op(uop)?),
            expr,
            None,
            false,
        ),
        Uop::Usilence => e.local_scope(|e| {
            let temp_local = e.local_gen_mut().get_unnamed();
            Ok(InstrSeq::gather(vec![
                emit_pos(pos),
                instr::silence_start(temp_local),
                {
                    let try_instrs = emit_expr(e, env, expr)?;
                    let catch_instrs =
                        InstrSeq::gather(vec![emit_pos(pos), instr::silence_end(temp_local)]);
                    scope::create_try_catch(
                        e.label_gen_mut(),
                        None,
                        false, /* skip_throw */
                        try_instrs,
                        catch_instrs,
                    )
                },
                emit_pos(pos),
                instr::silence_end(temp_local),
            ]))
        }),
    }
}

fn unop_to_incdec_op(op: &ast_defs::Uop) -> Result<IncDecOp> {
    use ast_defs::Uop;
    match op {
        Uop::Uincr => Ok(IncDecOp::PreInc),
        Uop::Udecr => Ok(IncDecOp::PreDec),
        Uop::Upincr => Ok(IncDecOp::PostInc),
        Uop::Updecr => Ok(IncDecOp::PostDec),
        _ => Err(Error::unrecoverable("invalid incdec op")),
    }
}

fn from_unop<'arena>(op: &ast_defs::Uop) -> Result<InstrSeq<'arena>> {
    use ast_defs::Uop;
    Ok(match op {
        Uop::Utild => instr::bit_not(),
        Uop::Unot => instr::not(),
        Uop::Uplus => instr::add(),
        Uop::Uminus => instr::sub(),
        _ => {
            return Err(Error::unrecoverable(
                "this unary operation cannot be translated",
            ));
        }
    })
}

fn binop_to_setopop(op: &ast_defs::Bop) -> Option<SetOpOp> {
    use ast_defs::Bop;
    match op {
        Bop::Plus => Some(SetOpOp::PlusEqual),
        Bop::Minus => Some(SetOpOp::MinusEqual),
        Bop::Star => Some(SetOpOp::MulEqual),
        Bop::Slash => Some(SetOpOp::DivEqual),
        Bop::Starstar => Some(SetOpOp::PowEqual),
        Bop::Amp => Some(SetOpOp::AndEqual),
        Bop::Bar => Some(SetOpOp::OrEqual),
        Bop::Xor => Some(SetOpOp::XorEqual),
        Bop::Ltlt => Some(SetOpOp::SlEqual),
        Bop::Gtgt => Some(SetOpOp::SrEqual),
        Bop::Percent => Some(SetOpOp::ModEqual),
        Bop::Dot => Some(SetOpOp::ConcatEqual),
        _ => None,
    }
}

fn optimize_null_checks<'arena, 'decl>(e: &Emitter<'arena, 'decl>) -> bool {
    e.options().compiler_flags.optimize_null_checks
}

fn from_binop<'arena>(op: &ast_defs::Bop) -> Result<InstrSeq<'arena>> {
    use ast_defs::Bop as B;
    Ok(match op {
        B::Plus => instr::add(),
        B::Minus => instr::sub(),
        B::Star => instr::mul(),
        B::Slash => instr::div(),
        B::Eqeq => instr::eq(),
        B::Eqeqeq => instr::same(),
        B::Starstar => instr::pow(),
        B::Diff => instr::neq(),
        B::Diff2 => instr::n_same(),
        B::Lt => instr::lt(),
        B::Lte => instr::lte(),
        B::Gt => instr::gt(),
        B::Gte => instr::gte(),
        B::Dot => instr::concat(),
        B::Amp => instr::bit_and(),
        B::Bar => instr::bit_or(),
        B::Ltlt => instr::shl(),
        B::Gtgt => instr::shr(),
        B::Cmp => instr::cmp(),
        B::Percent => instr::mod_(),
        B::Xor => instr::bit_xor(),
        B::Eq(_) => {
            return Err(Error::unrecoverable("assignment is emitted differently"));
        }
        B::QuestionQuestion => {
            return Err(Error::unrecoverable(
                "null coalescence is emitted differently",
            ));
        }
        B::Barbar | B::Ampamp => {
            return Err(Error::unrecoverable(
                "short-circuiting operator cannot be generated as a simple binop",
            ));
        }
    })
}

fn emit_first_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<(InstrSeq<'arena>, bool)> {
    Ok(match &expr.2 {
        ast::Expr_::Lvar(l)
            if !((is_local_this(env, &l.1) && !env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS))
                || superglobals::is_any_global(local_id::get_name(&l.1))) =>
        {
            (
                instr::c_get_l_2(get_local(e, env, &l.0, local_id::get_name(&l.1))?),
                true,
            )
        }
        _ => (emit_expr(e, env, expr)?, false),
    })
}

pub fn emit_two_exprs<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    e1: &ast::Expr,
    e2: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let (instrs1, is_under_top) = emit_first_expr(e, env, e1)?;
    let instrs2 = emit_expr(e, env, e2)?;
    let instrs2_is_var = e2.2.is_lvar();
    Ok(InstrSeq::gather(if is_under_top {
        if instrs2_is_var {
            vec![emit_pos(outer_pos), instrs2, instrs1]
        } else {
            vec![instrs2, emit_pos(outer_pos), instrs1]
        }
    } else if instrs2_is_var {
        vec![instrs1, emit_pos(outer_pos), instrs2]
    } else {
        vec![instrs1, instrs2, emit_pos(outer_pos)]
    }))
}

fn emit_readonly_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &aast::Expr<(), ()>,
) -> Result<InstrSeq<'arena>> {
    match &expr.2 {
        aast::Expr_::ObjGet(x) => {
            Ok(emit_obj_get(e, env, pos, QueryMOp::CGet, &x.0, &x.1, &x.2, false, true)?.0)
        }
        aast::Expr_::Call(c) => emit_call_expr(e, env, pos, None, true, c),
        aast::Expr_::ClassGet(x) => {
            emit_class_get(e, env, QueryMOp::CGet, &x.0, &x.1, ReadonlyOp::Any)
        }
        _ => emit_expr(e, env, expr),
    }
}

fn emit_quiet_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq<'arena>, Option<StackIndex>)> {
    match &expr.2 {
        ast::Expr_::Lvar(lid) if !is_local_this(env, &lid.1) => Ok((
            instr::c_get_quiet_l(get_local(e, env, pos, local_id::get_name(&lid.1))?),
            None,
        )),
        ast::Expr_::ArrayGet(x) => emit_array_get(
            e,
            env,
            pos,
            None,
            QueryMOp::CGetQuiet,
            &x.0,
            x.1.as_ref(),
            false,
            null_coalesce_assignment,
        ),
        ast::Expr_::ObjGet(x) if x.as_ref().3 == ast::PropOrMethod::IsProp => emit_obj_get(
            e,
            env,
            pos,
            QueryMOp::CGetQuiet,
            &x.0,
            &x.1,
            &x.2,
            null_coalesce_assignment,
            false,
        ),
        _ => Ok((emit_expr(e, env, expr)?, None)),
    }
}

fn emit_null_coalesce_assignment<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    e1: &ast::Expr,
    e2: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let end_label = e.label_gen_mut().next_regular();
    let do_set_label = e.label_gen_mut().next_regular();
    let l_nonnull = e.local_gen_mut().get_unnamed();
    let (quiet_instr, querym_n_unpopped) = emit_quiet_expr(e, env, pos, e1, true)?;
    let emit_popc_n = |n_unpopped| match n_unpopped {
        Some(n) => InstrSeq::gather(
            iter::repeat_with(instr::pop_c)
                .take(n as usize)
                .collect_vec(),
        ),
        None => instr::empty(),
    };
    Ok(InstrSeq::gather(vec![
        quiet_instr,
        instr::dup(),
        instr::is_type_c(IsTypeOp::Null),
        instr::jmp_nz(do_set_label),
        instr::pop_l(l_nonnull),
        emit_popc_n(querym_n_unpopped),
        instr::push_l(l_nonnull),
        instr::jmp(end_label),
        instr::label(do_set_label),
        instr::pop_c(),
        // null_coalesce_assignment is true so emit_lval_op will assume the
        // sources are already pushed on the stack.
        emit_lval_op(e, env, pos, LValOp::Set, e1, Some(e2), true)?,
        instr::label(end_label),
    ]))
}

fn emit_short_circuit_op<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let its_true = e.label_gen_mut().next_regular();
    let its_done = e.label_gen_mut().next_regular();
    let jmp_instrs = emit_jmpnz(e, env, expr, its_true)?;
    Ok(if jmp_instrs.is_fallthrough {
        InstrSeq::gather(vec![
            jmp_instrs.instrs,
            emit_pos(pos),
            instr::false_(),
            instr::jmp(its_done),
            if jmp_instrs.is_label_used {
                InstrSeq::gather(vec![instr::label(its_true), emit_pos(pos), instr::true_()])
            } else {
                instr::empty()
            },
            instr::label(its_done),
        ])
    } else {
        InstrSeq::gather(vec![
            jmp_instrs.instrs,
            if jmp_instrs.is_label_used {
                InstrSeq::gather(vec![instr::label(its_true), emit_pos(pos), instr::true_()])
            } else {
                instr::empty()
            },
        ])
    })
}

fn emit_binop<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let ast::Binop {
        bop: op,
        lhs: e1,
        rhs: e2,
    } = expr.2.as_binop().unwrap();
    use ast_defs::Bop as B;
    match op {
        B::Ampamp | B::Barbar => emit_short_circuit_op(e, env, pos, expr),
        B::Eq(None) => emit_lval_op(e, env, pos, LValOp::Set, e1, Some(e2), false),
        B::Eq(Some(eop)) if eop.is_question_question() => {
            emit_null_coalesce_assignment(e, env, pos, e1, e2)
        }
        B::Eq(Some(eop)) => match binop_to_setopop(eop) {
            None => Err(Error::unrecoverable("illegal eq op")),
            Some(op) => emit_lval_op(e, env, pos, LValOp::SetOp(op), e1, Some(e2), false),
        },
        B::QuestionQuestion => {
            let end_label = e.label_gen_mut().next_regular();
            let rhs = emit_expr(e, env, e2)?;
            Ok(InstrSeq::gather(vec![
                emit_quiet_expr(e, env, pos, e1, false)?.0,
                instr::dup(),
                instr::is_type_c(IsTypeOp::Null),
                instr::not(),
                instr::jmp_nz(end_label),
                instr::pop_c(),
                rhs,
                instr::label(end_label),
            ]))
        }
        _ => {
            let default = |e: &mut Emitter<'arena, 'decl>| {
                Ok(InstrSeq::gather(vec![
                    emit_two_exprs(e, env, pos, e1, e2)?,
                    from_binop(op)?,
                ]))
            };
            if optimize_null_checks(e) {
                match op {
                    B::Eqeqeq if e2.2.is_null() => emit_is_null(e, env, e1),
                    B::Eqeqeq if e1.2.is_null() => emit_is_null(e, env, e2),
                    B::Diff2 if e2.2.is_null() => Ok(InstrSeq::gather(vec![
                        emit_is_null(e, env, e1)?,
                        instr::not(),
                    ])),
                    B::Diff2 if e1.2.is_null() => Ok(InstrSeq::gather(vec![
                        emit_is_null(e, env, e2)?,
                        instr::not(),
                    ])),
                    _ => default(e),
                }
            } else {
                default(e)
            }
        }
    }
}

fn emit_pipe<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    (_, e1, e2): &(aast_defs::Lid, ast::Expr, ast::Expr),
) -> Result<InstrSeq<'arena>> {
    let lhs_instrs = emit_expr(e, env, e1)?;
    scope::with_unnamed_local(e, |e, local| {
        // TODO(hrust) avoid cloning env

        let mut pipe_env = env.clone();
        pipe_env.with_pipe_var(local);
        let rhs_instrs = emit_expr(e, &pipe_env, e2)?;
        Ok((
            InstrSeq::gather(vec![lhs_instrs, instr::pop_l(local)]),
            rhs_instrs,
            instr::empty(),
        ))
    })
}

fn emit_as<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    as_: &ast::As_,
) -> Result<InstrSeq<'arena>> {
    let ast::As_ {
        expr,
        hint,
        is_nullable,
        enforce_deep,
    } = as_;
    e.local_scope(|e| {
        let arg_local = e.local_gen_mut().get_unnamed();
        let type_struct_local = e.local_gen_mut().get_unnamed();
        let (ts_instrs, is_static) = emit_reified_arg(e, env, pos, true, hint)?;
        let then_label = e.label_gen_mut().next_regular();
        let done_label = e.label_gen_mut().next_regular();
        let (enforcement, exception) = if *enforce_deep {
            (
                hhbc::TypeStructEnforceKind::Deep,
                AsTypeStructExceptionKind::Typehint,
            )
        } else {
            (
                hhbc::TypeStructEnforceKind::Shallow,
                AsTypeStructExceptionKind::Error,
            )
        };
        let main_block = |ts_instrs, resolve| {
            InstrSeq::gather(vec![
                ts_instrs,
                instr::set_l(type_struct_local),
                match resolve {
                    TypeStructResolveOp::Resolve => {
                        instr::is_type_struct_c(hhbc::TypeStructResolveOp::Resolve, enforcement)
                    }
                    TypeStructResolveOp::DontResolve => {
                        instr::is_type_struct_c(hhbc::TypeStructResolveOp::DontResolve, enforcement)
                    }
                    _ => panic!("Enum value does not match one of listed variants"),
                },
                instr::jmp_nz(then_label),
                if *is_nullable {
                    InstrSeq::gather(vec![instr::null(), instr::jmp(done_label)])
                } else {
                    InstrSeq::gather(vec![
                        instr::push_l(arg_local),
                        instr::push_l(type_struct_local),
                        instr::throw_as_type_struct_exception(exception),
                    ])
                },
            ])
        };
        let i2 = if is_static {
            main_block(
                get_type_structure_for_hint(
                    e,
                    &[],
                    &IndexSet::new(),
                    TypeRefinementInHint::Disallowed,
                    hint,
                )?,
                TypeStructResolveOp::Resolve,
            )
        } else {
            main_block(ts_instrs, TypeStructResolveOp::DontResolve)
        };
        let i1 = emit_expr(e, env, expr)?;
        Ok(InstrSeq::gather(vec![
            i1,
            instr::set_l(arg_local),
            i2,
            instr::label(then_label),
            instr::push_l(arg_local),
            instr::unset_l(type_struct_local),
            instr::label(done_label),
        ]))
    })
}

fn emit_cast<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    hint: &aast_defs::Hint_,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    use aast_defs::Hint_ as H_;
    let op = match hint {
        H_::Happly(ast_defs::Id(_, id), hints) if hints.is_empty() => {
            let id = string_utils::strip_ns(id);
            match string_utils::strip_hh_ns(id).as_ref() {
                typehints::INT => instr::cast_int(),
                typehints::BOOL => instr::cast_bool(),
                typehints::STRING => instr::cast_string(),
                typehints::FLOAT => instr::cast_double(),
                _ => {
                    return Err(Error::fatal_parse(
                        pos,
                        format!("Invalid cast type: {}", id),
                    ));
                }
            }
        }
        _ => return Err(Error::fatal_parse(pos, "Invalid cast type")),
    };
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        emit_pos(pos),
        op,
    ]))
}

pub fn emit_unset_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    emit_lval_op_nonlist(
        e,
        env,
        &expr.1,
        LValOp::Unset,
        expr,
        instr::empty(),
        0,
        false,
        false,
    )
}

pub fn emit_set_range_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    name: &str,
    kind: SetRange,
    args: &[(ParamKind, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    let raise_fatal = |msg: &str| Err(Error::fatal_parse(pos, format!("{} {}", name, msg)));

    // TODO(hgoldstein) Weirdly enough, we *ignore* when the first argument is `inout`
    // unconditionally. We probably want to either _always_ require it or never require it.
    let ((_, base), (pk_offset, offset), (pk_src, src), args) = if args.len() >= 3 {
        (&args[0], &args[1], &args[2], &args[3..])
    } else {
        return raise_fatal("expects at least 3 arguments");
    };

    error::ensure_normal_paramkind(pk_offset)?;
    error::ensure_normal_paramkind(pk_src)?;

    let count_instrs = match (args, kind.vec) {
        ([c], true) => emit_expr(e, env, error::expect_normal_paramkind(c)?)?,
        ([], _) => instr::int(-1),
        (_, false) => return raise_fatal("expects no more than 3 arguments"),
        (_, true) => return raise_fatal("expects no more than 4 arguments"),
    };

    let (base_expr, cls_expr, base_setup, base_stack, cls_stack) = emit_base(
        e,
        env,
        base,
        MOpMode::Define,
        false, /* is_object */
        BareThisOp::Notice,
        false,           /*null_coalesce_assignment*/
        3,               /* base_offset */
        3,               /* rhs_stack_size */
        ReadonlyOp::Any, /* readonly_op */
    )?;
    Ok(InstrSeq::gather(vec![
        base_expr,
        cls_expr,
        emit_expr(e, env, offset)?,
        emit_expr(e, env, src)?,
        count_instrs,
        base_setup,
        instr::instr(Instruct::Opcode(Opcode::SetRangeM(
            base_stack + cls_stack,
            kind.size.try_into().expect("SetRange size overflow"),
            kind.op,
        ))),
    ]))
}

/// Emit code for a base expression `expr` that forms part of
/// an element access `expr[elem]` or field access `expr->fld`.
/// The instructions are divided into three sections:
///   1. base and element/property expression instructions:
///      push non-trivial base and key values on the stack
///   2. class instructions: emitted when the base is a static property access.
///      A sequence of instructions that pushes the property and the class on the
///      stack to be consumed by a BaseSC. (Foo::$bar)
///   3. base selector instructions: a sequence of Base/Dim instructions that
///      actually constructs the base address from "member keys" that are inlined
///      in the instructions, or pulled from the key values that
///      were pushed on the stack in section 1.
///   4. (constructed by the caller) a final accessor e.g. QueryM or setter
///      e.g. SetOpM instruction that has the final key inlined in the
///      instruction, or pulled from the key values that were pushed on the
///      stack in section 1.
///
/// The function returns a 5-tuple:
/// (base_instrs, cls_instrs, base_setup_instrs, base_stack_size, cls_stack_size)
/// where base_instrs is section 1 above, cls_instrs is section 2, base_setup_instrs
/// is section 3, stack_size is the number of values pushed on the stack by
/// section 1, and cls_stack_size is the number of values pushed on the stack by
/// section 2.
///
/// For example, the r-value expression $arr[3][$ix+2]
/// will compile to
///   # Section 1, pushing the value of $ix+2 on the stack
///   Int 2
///   CGetL2 $ix
///   Add
///   # Section 2, constructing the base address of $arr[3]
///   BaseL $arr Warn
///   Dim Warn EI:3
///   # Section 3, indexing the array using the value at stack position 0 (EC:0)
///   QueryM 1 CGet EC:0
fn emit_base<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    mode: MOpMode,
    is_object: bool,
    notice: BareThisOp,
    null_coalesce_assignment: bool,
    base_offset: StackIndex,
    rhs_stack_size: StackIndex,
    readonly_enforcement: ReadonlyOp, // this value depends on where we are emitting the base
) -> Result<(
    InstrSeq<'arena>,
    InstrSeq<'arena>,
    InstrSeq<'arena>,
    StackIndex,
    StackIndex,
)> {
    let result = emit_base_(
        e,
        env,
        expr,
        mode,
        is_object,
        notice,
        null_coalesce_assignment,
        base_offset,
        rhs_stack_size,
        None,
        readonly_enforcement,
    )?;
    match result {
        ArrayGetBase::Regular(i) => Ok((
            i.base_instrs,
            i.cls_instrs,
            i.setup_instrs,
            i.base_stack_size,
            i.cls_stack_size,
        )),
        ArrayGetBase::Inout { .. } => Err(Error::unrecoverable("unexpected input")),
    }
}

fn is_trivial(env: &Env<'_, '_>, is_base: bool, expr: &ast::Expr) -> bool {
    use ast::Expr_;
    match &expr.2 {
        Expr_::Int(_) | Expr_::String(_) => true,
        Expr_::Lvar(x) => {
            !is_local_this(env, &x.1) || env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS)
        }
        Expr_::ArrayGet(_) if !is_base => false,
        Expr_::ArrayGet(x) => {
            is_trivial(env, is_base, &x.0)
                && (x.1).as_ref().map_or(true, |e| is_trivial(env, is_base, e))
        }
        _ => false,
    }
}

fn get_local_temp_kind<'a, 'arena>(
    env: &Env<'a, 'arena>,
    is_base: bool,
    inout_param_info: Option<(usize, &inout_locals::AliasInfoMap<'_>)>,
    expr: Option<&ast::Expr>,
) -> Option<StoredValueKind> {
    match (expr, inout_param_info) {
        (_, None) => None,
        (Some(ast::Expr(_, _, ast::Expr_::Lvar(id))), Some((i, aliases)))
            if inout_locals::should_save_local_value(id.name(), i, aliases) =>
        {
            Some(StoredValueKind::Local)
        }
        (Some(e), _) => {
            if is_trivial(env, is_base, e) {
                None
            } else {
                Some(StoredValueKind::Expr)
            }
        }
        (None, _) => None,
    }
}

fn emit_base_<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    mode: MOpMode,
    is_object: bool,
    notice: BareThisOp,
    null_coalesce_assignment: bool,
    base_offset: StackIndex,
    rhs_stack_size: StackIndex,
    inout_param_info: Option<(usize, &inout_locals::AliasInfoMap<'_>)>,
    readonly_op: ReadonlyOp,
) -> Result<ArrayGetBase<'arena>> {
    let alloc = env.arena;
    let pos = &expr.1;
    let expr_ = &expr.2;
    let base_mode = if mode == MOpMode::InOut {
        MOpMode::Warn
    } else {
        mode
    };
    let local_temp_kind = get_local_temp_kind(env, true, inout_param_info, Some(expr));
    let emit_default = |e: &mut Emitter<'arena, 'decl>,
                        base_instrs,
                        cls_instrs,
                        setup_instrs,
                        base_stack_size,
                        cls_stack_size| {
        match local_temp_kind {
            Some(local_temp) => {
                let local = e.local_gen_mut().get_unnamed();
                ArrayGetBase::Inout {
                    load: ArrayGetBaseData {
                        base_instrs: vec![(base_instrs, Some((local, local_temp)))],
                        cls_instrs,
                        setup_instrs,
                        base_stack_size,
                        cls_stack_size,
                    },
                    store: instr::base_l(local, MOpMode::Define, ReadonlyOp::Any),
                }
            }
            _ => ArrayGetBase::Regular(ArrayGetBaseData {
                base_instrs,
                cls_instrs,
                setup_instrs,
                base_stack_size,
                cls_stack_size,
            }),
        }
    };
    // Called when emitting a base with MOpMode::Define on a Readonly expression
    let emit_readonly_lval_base = |e: &mut Emitter<'arena, 'decl>,
                                   env: &Env<'a, 'arena>,
                                   // expression inside of readonly expression
                                   inner_expr: &ast::Expr|
     -> Option<Result<ArrayGetBase<'_>>> {
        // Readonly local variable requires a CheckROCOW
        if let aast::Expr(_, _, Expr_::Lvar(x)) = inner_expr {
            if !is_local_this(env, &x.1) || env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) {
                match get_local(e, env, &x.0, &(x.1).1) {
                    Ok(v) => {
                        let base_instr = if local_temp_kind.is_some() {
                            instr::c_get_quiet_l(v)
                        } else {
                            instr::empty()
                        };
                        Some(Ok(emit_default(
                            e,
                            base_instr,
                            instr::empty(),
                            instr::base_l(v, MOpMode::Define, ReadonlyOp::CheckROCOW),
                            0,
                            0,
                        )))
                    }
                    Err(e) => Some(Err(e)),
                }
            } else {
                None // Found a local variable case that does not work
            }
        } else {
            // The only other reasonable case is if the inner expression
            // is a ClassGet, in which case we can use the default emit_base_ logic to handle
            None // Otherwise, ignore readonly
        }
    };

    let emit_expr_default =
        |e: &mut Emitter<'arena, 'decl>, env, expr: &ast::Expr| -> Result<ArrayGetBase<'_>> {
            let base_expr_instrs = emit_expr(e, env, expr)?;
            Ok(emit_default(
                e,
                base_expr_instrs,
                instr::empty(),
                emit_pos_then(pos, instr::base_c(base_offset, base_mode)),
                1,
                0,
            ))
        };

    use ast::Expr_;
    match expr_ {
        // Readonly expression in assignment. If null_coalesce_assignment is
        // true then it's REALLY important that we don't change how we set up
        // the stack for the initial query vs the null-sided assignment because
        // the caller assumes they're the same.
        Expr_::ReadonlyExpr(r) if base_mode == MOpMode::Define && !null_coalesce_assignment => {
            if let Some(result) = emit_readonly_lval_base(e, env, r) {
                result
            } else {
                // If we're not able to emit special readonly expression instructions, emit code as if readonly
                // expression does not exist
                emit_base_(
                    e,
                    env,
                    r,
                    mode,
                    is_object,
                    notice,
                    null_coalesce_assignment,
                    base_offset,
                    rhs_stack_size,
                    inout_param_info,
                    readonly_op,
                )
            }
        }
        Expr_::Lvar(x) if superglobals::is_superglobal(&(x.1).1) => {
            let base_instrs = emit_pos_then(
                &x.0,
                instr::string(alloc, string_utils::locals::strip_dollar(&(x.1).1)),
            );

            Ok(emit_default(
                e,
                base_instrs,
                instr::empty(),
                instr::base_gc(base_offset, base_mode),
                1,
                0,
            ))
        }
        Expr_::Lvar(x) if is_object && (x.1).1 == special_idents::THIS => {
            let base_instrs = emit_pos_then(&x.0, instr::check_this());
            Ok(emit_default(
                e,
                base_instrs,
                instr::empty(),
                instr::base_h(),
                0,
                0,
            ))
        }
        Expr_::Lvar(x)
            if !is_local_this(env, &x.1) || env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) =>
        {
            let v = get_local(e, env, &x.0, &(x.1).1)?;
            let base_instr = if local_temp_kind.is_some() {
                instr::c_get_quiet_l(v)
            } else {
                instr::empty()
            };
            Ok(emit_default(
                e,
                base_instr,
                instr::empty(),
                instr::base_l(v, base_mode, ReadonlyOp::Any),
                0,
                0,
            ))
        }
        Expr_::Lvar(lid) => {
            let local = emit_local(e, env, notice, lid)?;
            Ok(emit_default(
                e,
                local,
                instr::empty(),
                instr::base_c(base_offset, base_mode),
                1,
                0,
            ))
        }
        Expr_::ArrayGet(x) => match (&(x.0).1, x.1.as_ref()) {
            // $a[] can not be used as the base of an array get unless as an lval
            (_, None) if !env.flags.contains(env::Flags::ALLOWS_ARRAY_APPEND) => {
                Err(Error::fatal_runtime(pos, "Can't use [] for reading"))
            }
            // base is in turn array_get - do a specific handling for inout params
            // if necessary
            (_, opt_elem_expr) => {
                let base_expr = &x.0;
                let local_temp_kind =
                    get_local_temp_kind(env, false, inout_param_info, opt_elem_expr);
                let (elem_instrs, elem_stack_size) = emit_elem(
                    e,
                    env,
                    opt_elem_expr,
                    local_temp_kind,
                    null_coalesce_assignment,
                )?;
                let base_result = emit_base_(
                    e,
                    env,
                    base_expr,
                    mode,
                    false,
                    notice,
                    null_coalesce_assignment,
                    base_offset + elem_stack_size,
                    rhs_stack_size,
                    inout_param_info,
                    readonly_op, // continue passing readonly enforcement up
                )?;
                let cls_stack_size = match &base_result {
                    ArrayGetBase::Regular(base) => base.cls_stack_size,
                    ArrayGetBase::Inout { load, .. } => load.cls_stack_size,
                };
                let (mk, warninstr) = get_elem_member_key(
                    e,
                    env,
                    base_offset + cls_stack_size,
                    opt_elem_expr,
                    null_coalesce_assignment,
                )?;
                let make_setup_instrs = |base_setup_instrs: InstrSeq<'arena>| {
                    InstrSeq::gather(vec![warninstr, base_setup_instrs, instr::dim(mode, mk)])
                };
                Ok(match (base_result, local_temp_kind) {
                    // both base and index don't use temps - fallback to default handler
                    (ArrayGetBase::Regular(base), None) => emit_default(
                        e,
                        InstrSeq::gather(vec![base.base_instrs, elem_instrs]),
                        base.cls_instrs,
                        make_setup_instrs(base.setup_instrs),
                        base.base_stack_size + elem_stack_size,
                        base.cls_stack_size,
                    ),
                    // base does not need temps but index does
                    (ArrayGetBase::Regular(base), Some(local_temp)) => {
                        let local = e.local_gen_mut().get_unnamed();
                        let base_instrs = InstrSeq::gather(vec![base.base_instrs, elem_instrs]);
                        ArrayGetBase::Inout {
                            load: ArrayGetBaseData {
                                // store result of instr_begin to temp
                                base_instrs: vec![(base_instrs, Some((local, local_temp)))],
                                cls_instrs: base.cls_instrs,
                                setup_instrs: make_setup_instrs(base.setup_instrs),
                                base_stack_size: base.base_stack_size + elem_stack_size,
                                cls_stack_size: base.cls_stack_size,
                            },
                            store: emit_store_for_simple_base(
                                e,
                                env,
                                pos,
                                elem_stack_size,
                                base_expr,
                                local,
                                true,
                                readonly_op,
                            )?,
                        }
                    }
                    // base needs temps, index - does not
                    (
                        ArrayGetBase::Inout {
                            load:
                                ArrayGetBaseData {
                                    mut base_instrs,
                                    cls_instrs,
                                    setup_instrs,
                                    base_stack_size,
                                    cls_stack_size,
                                },
                            store,
                        },
                        None,
                    ) => {
                        base_instrs.push((elem_instrs, None));
                        ArrayGetBase::Inout {
                            load: ArrayGetBaseData {
                                base_instrs,
                                cls_instrs,
                                setup_instrs: make_setup_instrs(setup_instrs),
                                base_stack_size: base_stack_size + elem_stack_size,
                                cls_stack_size,
                            },
                            store: InstrSeq::gather(vec![store, instr::dim(MOpMode::Define, mk)]),
                        }
                    }
                    // both base and index needs locals
                    (
                        ArrayGetBase::Inout {
                            load:
                                ArrayGetBaseData {
                                    mut base_instrs,
                                    cls_instrs,
                                    setup_instrs,
                                    base_stack_size,
                                    cls_stack_size,
                                },
                            store,
                        },
                        Some(local_kind),
                    ) => {
                        let local = e.local_gen_mut().get_unnamed();
                        base_instrs.push((elem_instrs, Some((local, local_kind))));
                        ArrayGetBase::Inout {
                            load: ArrayGetBaseData {
                                base_instrs,
                                cls_instrs,
                                setup_instrs: make_setup_instrs(setup_instrs),
                                base_stack_size: base_stack_size + elem_stack_size,
                                cls_stack_size,
                            },
                            store: InstrSeq::gather(vec![
                                store,
                                instr::dim(MOpMode::Define, MemberKey::EL(local, ReadonlyOp::Any)),
                            ]),
                        }
                    }
                })
            }
        },
        Expr_::ObjGet(x) if x.as_ref().3 == ast::PropOrMethod::IsProp => {
            let (base_expr, prop_expr, null_flavor, _) = &**x;
            Ok(match prop_expr.2.as_id() {
                Some(ast_defs::Id(_, s)) if string_utils::is_xhp(s) => {
                    let base_instrs = emit_xhp_obj_get(e, env, pos, base_expr, s, null_flavor)?;
                    emit_default(
                        e,
                        base_instrs,
                        instr::empty(),
                        instr::base_c(base_offset, base_mode),
                        1,
                        0,
                    )
                }
                _ => {
                    let prop_stack_size = emit_prop_expr(
                        e,
                        env,
                        null_flavor,
                        0,
                        prop_expr,
                        null_coalesce_assignment,
                        ReadonlyOp::Any, // just getting stack size here
                    )?
                    .2;
                    let (
                        base_expr_instrs_begin,
                        base_expr_instrs_end,
                        base_setup_instrs,
                        base_stack_size,
                        cls_stack_size,
                    ) = emit_base(
                        e,
                        env,
                        base_expr,
                        mode,
                        true,
                        BareThisOp::Notice,
                        null_coalesce_assignment,
                        base_offset + prop_stack_size,
                        rhs_stack_size,
                        ReadonlyOp::Mutable, // the rest of the base must be completely mutable
                    )?;
                    let (mk, prop_instrs, _) = emit_prop_expr(
                        e,
                        env,
                        null_flavor,
                        base_offset + cls_stack_size,
                        prop_expr,
                        null_coalesce_assignment,
                        readonly_op, // use the current enforcement
                    )?;
                    let total_stack_size = prop_stack_size + base_stack_size;
                    let final_instr = instr::dim(mode, mk);
                    emit_default(
                        e,
                        InstrSeq::gather(vec![base_expr_instrs_begin, prop_instrs]),
                        base_expr_instrs_end,
                        InstrSeq::gather(vec![base_setup_instrs, final_instr]),
                        total_stack_size,
                        cls_stack_size,
                    )
                }
            })
        }
        Expr_::ClassGet(x) => {
            let (cid, prop, _) = &**x;
            let cexpr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, false, cid);
            let (cexpr_begin, cexpr_end) = emit_class_expr(e, env, cexpr, prop)?;
            Ok(emit_default(
                e,
                cexpr_begin,
                cexpr_end,
                instr::base_sc(base_offset + 1, rhs_stack_size, base_mode, readonly_op),
                1,
                1,
            ))
        }
        _ => emit_expr_default(e, env, expr),
    }
}

pub fn emit_ignored_exprs<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    exprs: &[ast::Expr],
) -> Result<InstrSeq<'arena>> {
    exprs
        .iter()
        .map(|e| emit_ignored_expr(emitter, env, pos, e))
        .collect::<Result<Vec<_>>>()
        .map(InstrSeq::gather)
}

pub fn emit_ignored_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![
        emit_expr(emitter, env, expr)?,
        emit_pos_then(pos, instr::pop_c()),
    ]))
}

fn emit_lval_op<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    op: LValOp,
    expr1: &ast::Expr,
    expr2: Option<&ast::Expr>,
    null_coalesce_assignment: bool,
) -> Result<InstrSeq<'arena>> {
    match (op, &expr1.2, expr2) {
        (LValOp::Set, ast::Expr_::List(l), Some(expr2)) => {
            let instr_rhs = emit_expr(e, env, expr2)?;
            let has_elements = l.iter().any(|e| !e.2.is_omitted());
            if !has_elements {
                Ok(instr_rhs)
            } else {
                scope::with_unnamed_local(e, |e, local| {
                    let loc = if can_use_as_rhs_in_list_assignment(&expr2.2)? {
                        Some(&local)
                    } else {
                        None
                    };
                    let (instr_lhs, instr_assign) = emit_lval_op_list(
                        e,
                        env,
                        pos,
                        loc,
                        &[],
                        expr1,
                        false,
                        is_readonly_expr(expr2),
                    )?;
                    Ok((
                        InstrSeq::gather(vec![instr_lhs, instr_rhs, instr::pop_l(local)]),
                        instr_assign,
                        instr::push_l(local),
                    ))
                })
            }
        }
        _ => e.local_scope(|e| {
            let (rhs_instrs, rhs_stack_size, rhs_readonly) = match expr2 {
                None => (instr::empty(), 0, false),
                Some(aast::Expr(_, _, aast::Expr_::Yield(af))) => {
                    let temp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            emit_yield(e, env, pos, af)?,
                            instr::set_l(temp),
                            instr::pop_c(),
                            instr::push_l(temp),
                        ]),
                        1,
                        false,
                    )
                }
                Some(expr) => (emit_expr(e, env, expr)?, 1, is_readonly_expr(expr)),
            };
            emit_lval_op_nonlist(
                e,
                env,
                pos,
                op,
                expr1,
                rhs_instrs,
                rhs_stack_size,
                rhs_readonly,
                null_coalesce_assignment,
            )
        }),
    }
}

fn can_use_as_rhs_in_list_assignment(expr: &ast::Expr_) -> Result<bool> {
    use aast::Expr_;
    Ok(match expr {
        Expr_::Call(box aast::CallExpr {
            func: ast::Expr(_, _, Expr_::Id(id)),
            ..
        }) if id.1 == special_functions::ECHO => false,
        Expr_::ObjGet(o) if o.as_ref().3 == ast::PropOrMethod::IsProp => true,
        Expr_::ClassGet(c) if c.as_ref().2 == ast::PropOrMethod::IsProp => true,
        Expr_::Lvar(_)
        | Expr_::ArrayGet(_)
        | Expr_::Call(_)
        | Expr_::FunctionPointer(_)
        | Expr_::New(_)
        | Expr_::Yield(_)
        | Expr_::Cast(_)
        | Expr_::Eif(_)
        | Expr_::Tuple(_)
        | Expr_::Collection(_)
        | Expr_::KeyValCollection(_)
        | Expr_::ValCollection(_)
        | Expr_::Clone(_)
        | Expr_::Unop(_)
        | Expr_::As(_)
        | Expr_::Upcast(_)
        | Expr_::Await(_)
        | Expr_::ReadonlyExpr(_)
        | Expr_::ClassConst(_) => true,
        Expr_::Pipe(p) => can_use_as_rhs_in_list_assignment(&(p.2).2)?,
        Expr_::Binop(b) => {
            if let ast_defs::Bop::Eq(None) = &b.bop {
                if (b.lhs).2.is_list() {
                    return can_use_as_rhs_in_list_assignment(&(b.rhs).2);
                }
            }
            b.bop.is_plus() || b.bop.is_question_question() || b.bop.is_any_eq()
        }
        _ => false,
    })
}

// Given a local $local and a list of integer array indices i_1, ..., i_n,
// generate code to extract the value of $local[i_n]...[i_1]:
//   BaseL $local Warn
//   Dim Warn EI:i_n ...
//   Dim Warn EI:i_2
//   QueryM 0 CGet EI:i_1
fn emit_array_get_fixed<'arena>(
    last_usage: bool,
    local: Local,
    indices: &[isize],
) -> InstrSeq<'arena> {
    let (base, stack_count) = if last_usage {
        (
            InstrSeq::gather(vec![instr::push_l(local), instr::base_c(0, MOpMode::Warn)]),
            1,
        )
    } else {
        (instr::base_l(local, MOpMode::Warn, ReadonlyOp::Any), 0)
    };
    let indices = InstrSeq::gather(
        indices
            .iter()
            .enumerate()
            .rev()
            .map(|(i, ix)| {
                let mk = MemberKey::EI(*ix as i64, ReadonlyOp::Any);
                if i == 0 {
                    instr::query_m(stack_count, QueryMOp::CGet, mk)
                } else {
                    instr::dim(MOpMode::Warn, mk)
                }
            })
            .collect(),
    );
    InstrSeq::gather(vec![base, indices])
}

/// Generate code for each lvalue assignment in a list destructuring expression.
/// Lvalues are assigned right-to-left, regardless of the nesting structure. So
///      list($a, list($b, $c)) = $d
///  and list(list($a, $b), $c) = $d
///  will both assign to $c, $b and $a in that order.
///  Returns a pair of instructions:
///  1. initialization part of the left hand side
///  2. assignment
///  this is necessary to handle cases like:
///  list($a[$f()]) = b();
///  here f() should be invoked before b()
pub fn emit_lval_op_list<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    local: Option<&Local>,
    indices: &[isize],
    expr: &ast::Expr,
    last_usage: bool,
    rhs_readonly: bool,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    use ast::Expr_;

    let is_ltr = e.options().hhbc.ltr_assign;
    match &expr.2 {
        Expr_::List(exprs) => {
            let last_non_omitted = if last_usage {
                // last usage of the local will happen when processing last non-omitted
                // element in the list - find it
                if is_ltr {
                    exprs.iter().rposition(|v| !v.2.is_omitted())
                } else {
                    // in right-to-left case result list will be reversed
                    // so we need to find first non-omitted expression
                    exprs.iter().rev().rposition(|v| !v.2.is_omitted())
                }
            } else {
                None
            };
            let (lhs_instrs, set_instrs): (Vec<InstrSeq<'arena>>, Vec<InstrSeq<'arena>>) = exprs
                .iter()
                .enumerate()
                .map(|(i, expr)| {
                    let mut new_indices = vec![i as isize];
                    new_indices.extend_from_slice(indices);
                    emit_lval_op_list(
                        e,
                        env,
                        outer_pos,
                        local,
                        &new_indices[..],
                        expr,
                        last_non_omitted.map_or(false, |j| j == i),
                        rhs_readonly,
                    )
                })
                .collect::<Result<Vec<_>>>()?
                .into_iter()
                .unzip();
            Ok((
                InstrSeq::gather(lhs_instrs),
                InstrSeq::gather(if !is_ltr {
                    set_instrs.into_iter().rev().collect()
                } else {
                    set_instrs
                }),
            ))
        }
        Expr_::Omitted => Ok((instr::empty(), instr::empty())),
        _ => {
            // Generate code to access the element from the array
            let access_instrs = match (local, indices) {
                (Some(loc), [_, ..]) => emit_array_get_fixed(last_usage, loc.to_owned(), indices),
                (Some(loc), []) => {
                    if last_usage {
                        instr::push_l(loc.to_owned())
                    } else {
                        instr::c_get_l(loc.to_owned())
                    }
                }
                (None, _) => instr::null(),
            };
            // Generate code to assign to the lvalue *)
            // Return pair: side effects to initialize lhs + assignment
            let (lhs_instrs, rhs_instrs, set_op) = emit_lval_op_nonlist_steps(
                e,
                env,
                outer_pos,
                LValOp::Set,
                expr,
                access_instrs,
                1,
                rhs_readonly,
                false,
            )?;
            Ok(if is_ltr {
                (
                    instr::empty(),
                    InstrSeq::gather(vec![lhs_instrs, rhs_instrs, set_op, instr::pop_c()]),
                )
            } else {
                (
                    lhs_instrs,
                    InstrSeq::gather(vec![instr::empty(), rhs_instrs, set_op, instr::pop_c()]),
                )
            })
        }
    }
}

pub fn emit_lval_op_nonlist<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    op: LValOp,
    expr: &ast::Expr,
    rhs_instrs: InstrSeq<'arena>,
    rhs_stack_size: u32,
    rhs_readonly: bool,
    null_coalesce_assignment: bool,
) -> Result<InstrSeq<'arena>> {
    emit_lval_op_nonlist_steps(
        e,
        env,
        outer_pos,
        op,
        expr,
        rhs_instrs,
        rhs_stack_size,
        rhs_readonly,
        null_coalesce_assignment,
    )
    .map(|(lhs, rhs, setop)| InstrSeq::gather(vec![lhs, rhs, setop]))
}

pub fn emit_final_global_op<'arena>(pos: &Pos, op: LValOp) -> InstrSeq<'arena> {
    use LValOp as L;
    match op {
        L::Set => emit_pos_then(pos, instr::set_g()),
        L::SetOp(op) => instr::set_op_g(op),
        L::IncDec(op) => instr::inc_dec_g(op),
        L::Unset => emit_pos_then(pos, instr::unset_g()),
    }
}

pub fn emit_final_local_op<'arena>(pos: &Pos, op: LValOp, lid: Local) -> InstrSeq<'arena> {
    use LValOp as L;
    emit_pos_then(
        pos,
        match op {
            L::Set => instr::set_l(lid),
            L::SetOp(op) => instr::set_op_l(lid, op),
            L::IncDec(op) => instr::inc_dec_l(lid, op),
            L::Unset => instr::unset_l(lid),
        },
    )
}

fn emit_final_member_op<'arena>(
    stack_size: StackIndex,
    op: LValOp,
    mk: MemberKey<'arena>,
) -> InstrSeq<'arena> {
    use LValOp as L;
    match op {
        L::Set => instr::set_m(stack_size, mk),
        L::SetOp(op) => instr::set_op_m(stack_size, op, mk),
        L::IncDec(op) => instr::inc_dec_m(stack_size, op, mk),
        L::Unset => instr::unset_m(stack_size, mk),
    }
}

fn emit_final_static_op<'arena>(
    alloc: &'arena bumpalo::Bump,
    cid: &ast::ClassId,
    prop: &ast::ClassGetExpr,
    op: LValOp,
) -> Result<InstrSeq<'arena>> {
    use LValOp as L;
    Ok(match op {
        L::Set => instr::set_s(ReadonlyOp::Any),
        L::SetOp(op) => instr::set_op_s(op),
        L::IncDec(op) => instr::inc_dec_s(op),
        L::Unset => {
            let pos = match prop {
                ast::ClassGetExpr::CGstring((pos, _))
                | ast::ClassGetExpr::CGexpr(ast::Expr(_, pos, _)) => pos,
            };
            let cid = text_of_class_id(cid);
            let id = text_of_prop(prop);
            emit_fatal::emit_fatal_runtime(
                alloc,
                pos,
                format!(
                    "Attempt to unset static property {}::{}",
                    string_utils::strip_ns(&cid),
                    id,
                ),
            )
        }
    })
}

pub fn emit_lval_op_nonlist_steps<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    op: LValOp,
    expr: &ast::Expr,
    rhs_instrs: InstrSeq<'arena>,
    rhs_stack_size: StackIndex,
    rhs_readonly: bool,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>)> {
    let f = |alloc: &'arena bumpalo::Bump, env: &mut Env<'a, 'arena>| {
        use ast::Expr_;
        let pos = &expr.1;
        Ok(match &expr.2 {
            Expr_::Lvar(v) if superglobals::is_any_global(local_id::get_name(&v.1)) => (
                emit_pos_then(
                    &v.0,
                    instr::string(alloc, string_utils::lstrip(local_id::get_name(&v.1), "$")),
                ),
                rhs_instrs,
                emit_final_global_op(outer_pos, op),
            ),
            Expr_::Lvar(v) if is_local_this(env, &v.1) && op.is_incdec() => (
                emit_local(e, env, BareThisOp::Notice, v)?,
                rhs_instrs,
                instr::empty(),
            ),
            Expr_::Lvar(v) if !is_local_this(env, &v.1) || op == LValOp::Unset => {
                (instr::empty(), rhs_instrs, {
                    let lid = get_local(e, env, &v.0, &(v.1).1)?;
                    emit_final_local_op(outer_pos, op, lid)
                })
            }
            Expr_::ArrayGet(x) => match x.1.as_ref() {
                None if !env.flags.contains(env::Flags::ALLOWS_ARRAY_APPEND) => {
                    return Err(Error::fatal_runtime(pos, "Can't use [] for reading"));
                }
                opt_elem_expr => {
                    let mode = match op {
                        LValOp::Unset => MOpMode::Unset,
                        _ => MOpMode::Define,
                    };
                    let (elem_instrs, elem_stack_size) =
                        emit_elem(e, env, opt_elem_expr, None, null_coalesce_assignment)?;
                    let base_offset = elem_stack_size + rhs_stack_size;
                    let readonly_op = if rhs_readonly {
                        ReadonlyOp::CheckROCOW // writing a readonly value requires a readonly copy on write array
                    } else {
                        ReadonlyOp::CheckMutROCOW // writing a mut value requires left side to be mutable or a ROCOW
                    };
                    let (
                        base_expr_instrs_begin,
                        base_expr_instrs_end,
                        base_setup_instrs,
                        base_stack_size,
                        cls_stack_size,
                    ) = emit_base(
                        e,
                        env,
                        &x.0,
                        mode,
                        false,
                        BareThisOp::Notice,
                        null_coalesce_assignment,
                        base_offset,
                        rhs_stack_size,
                        readonly_op,
                    )?;
                    let (mk, warninstr) = get_elem_member_key(
                        e,
                        env,
                        rhs_stack_size + cls_stack_size,
                        opt_elem_expr,
                        null_coalesce_assignment,
                    )?;
                    let total_stack_size = elem_stack_size + base_stack_size + cls_stack_size;
                    let final_instr =
                        emit_pos_then(pos, emit_final_member_op(total_stack_size, op, mk));
                    (
                        // Don't emit instructions for elems as these were not popped from
                        // the stack by the final member op during the lookup of a null
                        // coalesce assignment.
                        if null_coalesce_assignment {
                            instr::empty()
                        } else {
                            InstrSeq::gather(vec![
                                base_expr_instrs_begin,
                                elem_instrs,
                                base_expr_instrs_end,
                            ])
                        },
                        rhs_instrs,
                        InstrSeq::gather(vec![
                            emit_pos(pos),
                            warninstr,
                            base_setup_instrs,
                            final_instr,
                        ]),
                    )
                }
            },
            Expr_::ObjGet(x) if x.as_ref().3 == ast::PropOrMethod::IsProp => {
                let (e1, e2, nullflavor, _) = &**x;
                if nullflavor.eq(&ast_defs::OgNullFlavor::OGNullsafe) {
                    return Err(Error::fatal_parse(
                        pos,
                        "?-> is not allowed in write context",
                    ));
                }
                let mode = match op {
                    LValOp::Unset => MOpMode::Unset,
                    _ => MOpMode::Define,
                };
                let readonly_op = if rhs_readonly {
                    ReadonlyOp::Readonly
                } else {
                    ReadonlyOp::Any
                };
                let prop_stack_size = emit_prop_expr(
                    e,
                    env,
                    nullflavor,
                    0,
                    e2,
                    null_coalesce_assignment,
                    readonly_op,
                )?
                .2;
                let base_offset = prop_stack_size + rhs_stack_size;
                let (
                    base_expr_instrs_begin,
                    base_expr_instrs_end,
                    base_setup_instrs,
                    base_stack_size,
                    cls_stack_size,
                ) = emit_base(
                    e,
                    env,
                    e1,
                    mode,
                    true,
                    BareThisOp::Notice,
                    null_coalesce_assignment,
                    base_offset,
                    rhs_stack_size,
                    ReadonlyOp::Mutable, // writing to a property requires everything in the base to be mutable
                )?;
                let (mk, prop_instrs, _) = emit_prop_expr(
                    e,
                    env,
                    nullflavor,
                    rhs_stack_size + cls_stack_size,
                    e2,
                    null_coalesce_assignment,
                    readonly_op,
                )?;
                let total_stack_size = prop_stack_size + base_stack_size + cls_stack_size;
                let final_instr =
                    emit_pos_then(pos, emit_final_member_op(total_stack_size, op, mk));
                (
                    // Don't emit instructions for props as these were not popped from
                    // the stack by the final member op during the lookup of a null
                    // coalesce assignment.
                    if null_coalesce_assignment {
                        instr::empty()
                    } else {
                        InstrSeq::gather(vec![
                            base_expr_instrs_begin,
                            prop_instrs,
                            base_expr_instrs_end,
                        ])
                    },
                    rhs_instrs,
                    InstrSeq::gather(vec![base_setup_instrs, final_instr]),
                )
            }
            Expr_::ClassGet(x) if x.as_ref().2 == ast::PropOrMethod::IsProp => {
                let (cid, prop, _) = &**x;
                let cexpr = ClassExpr::class_id_to_class_expr(e, &env.scope, false, false, cid);
                let final_instr_ = emit_final_static_op(alloc, cid, prop, op)?;
                let final_instr = emit_pos_then(pos, final_instr_);
                let (cexpr_seq1, cexpr_seq2) = emit_class_expr(e, env, cexpr, prop)?;
                (
                    InstrSeq::gather(vec![cexpr_seq1, cexpr_seq2]),
                    rhs_instrs,
                    final_instr,
                )
            }
            Expr_::Unop(uop) => (
                instr::empty(),
                rhs_instrs,
                InstrSeq::gather(vec![
                    emit_lval_op_nonlist(
                        e,
                        env,
                        pos,
                        op,
                        &uop.1,
                        instr::empty(),
                        rhs_stack_size,
                        false,
                        false, // all unary operations (++, --, etc) are on primitives, so no HHVM readonly checks
                    )?,
                    from_unop(&uop.0)?,
                ]),
            ),
            _ => {
                return Err(Error::fatal_parse(
                    pos,
                    "Can't use return value in write context",
                ));
            }
        })
    };
    // TODO(shiqicao): remove clone!
    let alloc = env.arena;
    let mut env = env.clone();
    match op {
        LValOp::Set | LValOp::SetOp(_) | LValOp::IncDec(_) => {
            env.with_allows_array_append(alloc, f)
        }
        _ => f(alloc, &mut env),
    }
}

fn emit_class_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    cexpr: ClassExpr,
    prop: &ast::ClassGetExpr,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    let load_prop = |e: &mut Emitter<'arena, 'decl>| match prop {
        ast::ClassGetExpr::CGstring((pos, id)) => Ok(emit_pos_then(
            pos,
            instr::string(e.alloc, string_utils::locals::strip_dollar(id)),
        )),
        ast::ClassGetExpr::CGexpr(expr) => emit_expr(e, env, expr),
    };

    Ok(match &cexpr {
        ClassExpr::Expr(expr)
            if expr.2.is_call()
                || expr.2.is_binop()
                || expr.2.is_class_get()
                || expr
                    .2
                    .as_lvar()
                    .map_or(false, |ast::Lid(_, id)| local_id::get_name(id) == "$this") =>
        {
            let cexpr_local = emit_expr(e, env, expr)?;
            (
                instr::empty(),
                InstrSeq::gather(vec![
                    cexpr_local,
                    scope::stash_top_in_unnamed_local(e, load_prop)?,
                    instr::class_get_c(ClassGetCMode::Normal),
                ]),
            )
        }
        _ => {
            let pos = match prop {
                ast::ClassGetExpr::CGstring((pos, _))
                | ast::ClassGetExpr::CGexpr(ast::Expr(_, pos, _)) => pos,
            };
            (load_prop(e)?, emit_load_class_ref(e, env, pos, cexpr)?)
        }
    })
}

fn fixup_type_arg<'a, 'b, 'arena>(
    env: &Env<'b, 'arena>,
    isas: bool,
    hint: &'a ast::Hint,
) -> Result<Cow<'a, ast::Hint>> {
    struct Checker<'s> {
        erased_tparams: &'s [&'s str],
        isas: bool,
    }
    impl<'ast, 's> Visitor<'ast> for Checker<'s> {
        type Params = AstParams<(), Option<Error>>;

        fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
            self
        }

        fn visit_hint_fun(&mut self, c: &mut (), hf: &ast::HintFun) -> Result<(), Option<Error>> {
            hf.param_tys.accept(c, self.object())?;
            hf.return_ty.accept(c, self.object())
        }

        fn visit_hint(&mut self, c: &mut (), h: &ast::Hint) -> Result<(), Option<Error>> {
            use ast::Hint_ as H_;
            use ast::Id;
            match h.1.as_ref() {
                H_::Happly(Id(_, id), _)
                    if self.erased_tparams.contains(&id.as_str()) && self.isas =>
                {
                    return Err(Some(Error::fatal_parse(
                        &h.0,
                        "Erased generics are not allowed in is/as expressions",
                    )));
                }
                H_::Haccess(_, _) => return Ok(()),
                _ => {}
            }
            h.recurse(c, self.object())
        }

        fn visit_hint_(&mut self, c: &mut (), h: &ast::Hint_) -> Result<(), Option<Error>> {
            use ast::Hint_ as H_;
            use ast::Id;
            match h {
                H_::Happly(Id(_, id), _) if self.erased_tparams.contains(&id.as_str()) => Err(None),
                _ => h.recurse(c, self.object()),
            }
        }
    }

    struct Updater<'s> {
        erased_tparams: &'s [&'s str],
    }
    impl<'ast, 's> VisitorMut<'ast> for Updater<'s> {
        type Params = AstParams<(), ()>;

        fn object(&mut self) -> &mut dyn VisitorMut<'ast, Params = Self::Params> {
            self
        }

        fn visit_hint_fun(&mut self, c: &mut (), hf: &mut ast::HintFun) -> Result<(), ()> {
            <Vec<ast::Hint> as NodeMut<Self::Params>>::accept(&mut hf.param_tys, c, self.object())?;
            <ast::Hint as NodeMut<Self::Params>>::accept(&mut hf.return_ty, c, self.object())
        }

        fn visit_hint_(&mut self, c: &mut (), h: &mut ast::Hint_) -> Result<(), ()> {
            use ast::Hint_ as H_;
            use ast::Id;
            match h {
                H_::Happly(Id(_, id), _) if self.erased_tparams.contains(&id.as_str()) => {
                    Ok(*h = H_::Hwildcard)
                }
                _ => h.recurse(c, self.object()),
            }
        }
    }
    let erased_tparams = get_erased_tparams(env);
    let erased_tparams = erased_tparams.as_slice();
    let mut checker = Checker {
        erased_tparams,
        isas,
    };
    match visit(&mut checker, &mut (), hint) {
        Ok(()) => Ok(Cow::Borrowed(hint)),
        Err(Some(error)) => Err(error),
        Err(None) => {
            let mut updater = Updater { erased_tparams };
            let mut hint = hint.clone();
            visit_mut(&mut updater, &mut (), &mut hint).unwrap();
            Ok(Cow::Owned(hint))
        }
    }
}

pub fn emit_reified_arg<'b, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'b, 'arena>,
    pos: &Pos,
    isas: bool,
    hint: &ast::Hint,
) -> Result<(InstrSeq<'arena>, bool)> {
    struct Collector<'ast, 'a> {
        current_tags: &'a HashSet<&'a str>,
        acc: IndexSet<&'ast str>,
    }

    impl<'ast, 'a> Collector<'ast, 'a> {
        fn add_name(&mut self, name: &'ast str) {
            if self.current_tags.contains(name) && !self.acc.contains(name) {
                self.acc.insert(name);
            }
        }
    }

    impl<'ast, 'a> Visitor<'ast> for Collector<'ast, 'a> {
        type Params = AstParams<(), ()>;

        fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
            self
        }

        fn visit_hint_(&mut self, c: &mut (), h_: &'ast ast::Hint_) -> Result<(), ()> {
            use ast::Hint_ as H_;
            use ast::Id;
            match h_ {
                H_::Haccess(_, sids) => Ok(sids.iter().for_each(|Id(_, name)| self.add_name(name))),
                H_::Habstr(name, h) | H_::Happly(Id(_, name), h) => {
                    self.add_name(name);
                    h.accept(c, self.object())
                }
                _ => h_.recurse(c, self.object()),
            }
        }
    }
    let hint = fixup_type_arg(env, isas, hint)?;
    fn f<'a>(mut acc: HashSet<&'a str>, tparam: &'a ast::Tparam) -> HashSet<&'a str> {
        if tparam.reified != ast::ReifyKind::Erased {
            acc.insert(&tparam.name.1);
        }
        acc
    }
    let current_tags = env
        .scope
        .get_fun_tparams()
        .iter()
        .fold(HashSet::<&str>::default(), f);
    let class_tparams = env.scope.get_class_tparams();
    let current_tags = class_tparams.iter().fold(current_tags, f);
    let mut collector = Collector {
        current_tags: &current_tags,
        acc: IndexSet::new(),
    };
    visit(&mut collector, &mut (), &hint as &ast::Hint).unwrap();
    match hint.1.as_ref() {
        ast::Hint_::Happly(ast::Id(_, name), hs)
            if hs.is_empty() && current_tags.contains(name.as_str()) =>
        {
            Ok((emit_reified_type(e, env, pos, name)?, false))
        }
        _ => {
            let type_refinement_in_hint = if isas {
                TypeRefinementInHint::Disallowed
            } else {
                TypeRefinementInHint::Allowed
            };
            let ts = get_type_structure_for_hint(
                e,
                &[],
                &collector.acc,
                type_refinement_in_hint,
                &hint,
            )?;
            let ts_list = if collector.acc.is_empty() {
                ts
            } else {
                let values = collector
                    .acc
                    .iter()
                    .map(|v| emit_reified_type(e, env, pos, v))
                    .collect::<Result<Vec<_>>>()?;
                InstrSeq::gather(vec![InstrSeq::gather(values), ts])
            };
            Ok((
                InstrSeq::gather(vec![
                    ts_list,
                    instr::combine_and_resolve_type_struct(collector.acc.len() as u32 + 1),
                ]),
                collector.acc.is_empty(),
            ))
        }
    }
}

pub fn get_local<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    s: &str,
) -> Result<Local> {
    if s == special_idents::DOLLAR_DOLLAR {
        match &env.pipe_var {
            None => Err(Error::fatal_runtime(
                pos,
                "Pipe variables must occur only in the RHS of pipe expressions",
            )),
            Some(var) => Ok(*var),
        }
    } else if special_idents::is_tmp_var(s) {
        Ok(*e.local_gen().get_unnamed_for_tempname(s))
    } else {
        Ok(e.named_local(s.into()))
    }
}

pub fn emit_is_null<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    if let Some(ast::Lid(pos, id)) = expr.2.as_lvar() {
        if !is_local_this(env, id) {
            return Ok(instr::is_type_l(
                get_local(e, env, pos, local_id::get_name(id))?,
                IsTypeOp::Null,
            ));
        }
    }

    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        instr::is_type_c(IsTypeOp::Null),
    ]))
}

pub fn emit_jmpnz<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    label: Label,
) -> Result<EmitJmpResult<'arena>> {
    let ast::Expr(_, pos, expr_) = expr;
    let opt = optimize_null_checks(e);
    Ok(
        match constant_folder::expr_to_typed_value(e, &env.scope, expr) {
            Ok(tv) => {
                if constant_folder::cast_to_bool(tv) {
                    EmitJmpResult {
                        instrs: emit_pos_then(pos, instr::jmp(label)),
                        is_fallthrough: false,
                        is_label_used: true,
                    }
                } else {
                    EmitJmpResult {
                        instrs: emit_pos_then(pos, instr::empty()),
                        is_fallthrough: true,
                        is_label_used: false,
                    }
                }
            }
            Err(_) => {
                use ast::Expr_;
                use ast_defs::Uop;
                match expr_ {
                    Expr_::Unop(uo) if uo.0 == Uop::Unot => emit_jmpz(e, env, &uo.1, label)?,
                    Expr_::Binop(bo) if bo.bop.is_barbar() => {
                        let r1 = emit_jmpnz(e, env, &bo.lhs, label)?;
                        if r1.is_fallthrough {
                            let r2 = emit_jmpnz(e, env, &bo.rhs, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(vec![r1.instrs, r2.instrs]),
                                ),
                                is_fallthrough: r2.is_fallthrough,
                                is_label_used: r1.is_label_used || r2.is_label_used,
                            }
                        } else {
                            r1
                        }
                    }
                    Expr_::Binop(bo) if bo.bop.is_ampamp() => {
                        let skip_label = e.label_gen_mut().next_regular();
                        let r1 = emit_jmpz(e, env, &bo.lhs, skip_label)?;
                        if !r1.is_fallthrough {
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(if r1.is_label_used {
                                        vec![r1.instrs, instr::label(skip_label)]
                                    } else {
                                        vec![r1.instrs]
                                    }),
                                ),
                                is_fallthrough: r1.is_label_used,
                                is_label_used: false,
                            }
                        } else {
                            let r2 = emit_jmpnz(e, env, &bo.rhs, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(if r1.is_label_used {
                                        vec![r1.instrs, r2.instrs, instr::label(skip_label)]
                                    } else {
                                        vec![r1.instrs, r2.instrs]
                                    }),
                                ),
                                is_fallthrough: r2.is_fallthrough || r1.is_label_used,
                                is_label_used: r2.is_label_used,
                            }
                        }
                    }
                    Expr_::Binop(bo)
                        if bo.bop.is_eqeqeq()
                            && ((bo.lhs).2.is_null() || (bo.rhs).2.is_null())
                            && opt =>
                    {
                        let is_null = emit_is_null(
                            e,
                            env,
                            if (bo.lhs).2.is_null() {
                                &bo.rhs
                            } else {
                                &bo.lhs
                            },
                        )?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![is_null, instr::jmp_nz(label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    Expr_::Binop(bo)
                        if bo.bop.is_diff2()
                            && ((bo.lhs).2.is_null() || (bo.rhs).2.is_null())
                            && opt =>
                    {
                        let is_null = emit_is_null(
                            e,
                            env,
                            if (bo.lhs).2.is_null() {
                                &bo.rhs
                            } else {
                                &bo.lhs
                            },
                        )?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![is_null, instr::jmp_z(label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    _ => {
                        let instr = emit_expr(e, env, expr)?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![instr, instr::jmp_nz(label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                }
            }
        },
    )
}

pub fn emit_jmpz<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    label: Label,
) -> Result<EmitJmpResult<'arena>> {
    let ast::Expr(_, pos, expr_) = expr;
    let opt = optimize_null_checks(e);
    Ok(
        match constant_folder::expr_to_typed_value(e, &env.scope, expr) {
            Ok(v) => {
                if constant_folder::cast_to_bool(v) {
                    EmitJmpResult {
                        instrs: emit_pos_then(pos, instr::empty()),
                        is_fallthrough: true,
                        is_label_used: false,
                    }
                } else {
                    EmitJmpResult {
                        instrs: emit_pos_then(pos, instr::jmp(label)),
                        is_fallthrough: false,
                        is_label_used: true,
                    }
                }
            }
            Err(_) => {
                use ast::Expr_;
                use ast_defs::Uop;
                match expr_ {
                    Expr_::Unop(uo) if uo.0 == Uop::Unot => emit_jmpnz(e, env, &uo.1, label)?,
                    Expr_::Binop(bo) if bo.bop.is_barbar() => {
                        let skip_label = e.label_gen_mut().next_regular();
                        let r1 = emit_jmpnz(e, env, &bo.lhs, skip_label)?;
                        if !r1.is_fallthrough {
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(if r1.is_label_used {
                                        vec![r1.instrs, instr::label(skip_label)]
                                    } else {
                                        vec![r1.instrs]
                                    }),
                                ),
                                is_fallthrough: r1.is_label_used,
                                is_label_used: false,
                            }
                        } else {
                            let r2 = emit_jmpz(e, env, &bo.rhs, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(if r1.is_label_used {
                                        vec![r1.instrs, r2.instrs, instr::label(skip_label)]
                                    } else {
                                        vec![r1.instrs, r2.instrs]
                                    }),
                                ),
                                is_fallthrough: r2.is_fallthrough || r1.is_label_used,
                                is_label_used: r2.is_label_used,
                            }
                        }
                    }
                    Expr_::Binop(bo) if bo.bop.is_ampamp() => {
                        let r1 = emit_jmpz(e, env, &bo.lhs, label)?;
                        if r1.is_fallthrough {
                            let r2 = emit_jmpz(e, env, &bo.rhs, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(vec![r1.instrs, r2.instrs]),
                                ),
                                is_fallthrough: r2.is_fallthrough,
                                is_label_used: r1.is_label_used || r2.is_label_used,
                            }
                        } else {
                            EmitJmpResult {
                                instrs: emit_pos_then(pos, r1.instrs),
                                is_fallthrough: false,
                                is_label_used: r1.is_label_used,
                            }
                        }
                    }
                    Expr_::Binop(bo)
                        if bo.bop.is_eqeqeq()
                            && ((bo.lhs).2.is_null() || (bo.rhs).2.is_null())
                            && opt =>
                    {
                        let is_null = emit_is_null(
                            e,
                            env,
                            if (bo.lhs).2.is_null() {
                                &bo.rhs
                            } else {
                                &bo.lhs
                            },
                        )?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![is_null, instr::jmp_z(label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    Expr_::Binop(bo)
                        if bo.bop.is_diff2()
                            && ((bo.lhs).2.is_null() || (bo.rhs).2.is_null())
                            && opt =>
                    {
                        let is_null = emit_is_null(
                            e,
                            env,
                            if (bo.lhs).2.is_null() {
                                &bo.rhs
                            } else {
                                &bo.lhs
                            },
                        )?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![is_null, instr::jmp_nz(label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    _ => {
                        let instr = emit_expr(e, env, expr)?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![instr, instr::jmp_z(label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                }
            }
        },
    )
}

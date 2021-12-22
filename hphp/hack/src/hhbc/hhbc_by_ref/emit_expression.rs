// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Maybe::Just, Slice, Str};
use hhbc_by_ref_ast_class_expr::ClassExpr;
use hhbc_by_ref_ast_constant_folder as ast_constant_folder;
use hhbc_by_ref_emit_adata as emit_adata;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_emit_pos::{emit_pos, emit_pos_then};
use hhbc_by_ref_emit_symbol_refs as emit_symbol_refs;
use hhbc_by_ref_emit_type_constant as emit_type_constant;
use hhbc_by_ref_env::{emitter::Emitter, Env, Flags as EnvFlags};
use hhbc_by_ref_hhbc_assertion_utils::*;
use hhbc_by_ref_hhbc_ast::*;
use hhbc_by_ref_hhbc_id::{class, r#const, function, method, prop, Id};
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{
    instr, unrecoverable,
    Error::{self, Unrecoverable},
    InstrSeq, Result,
};
use hhbc_by_ref_label::Label;
use hhbc_by_ref_local::Local;
use hhbc_by_ref_options::{CompilerFlags, HhvmFlags, LangFlags, Options};
use hhbc_by_ref_runtime::TypedValue;
use hhbc_by_ref_scope::scope;
use hhbc_by_ref_symbol_refs_state::IncludePath;
use itertools::Either;
use lazy_static::lazy_static;
use naming_special_names_rust::{
    emitter_special_functions, fb, pseudo_consts, pseudo_functions, special_functions,
    special_idents, superglobals, typehints, user_attributes,
};
use oxidized::{
    aast, aast_defs,
    aast_visitor::{visit, visit_mut, AstParams, Node, NodeMut, Visitor, VisitorMut},
    ast,
    ast_defs::{self, ParamKind},
    local_id,
    pos::Pos,
};
use regex::Regex;

use hash::HashSet;
use indexmap::IndexSet;
use std::{collections::BTreeMap, iter, result::Result as StdResult, str::FromStr};

#[derive(Debug)]
pub struct EmitJmpResult<'arena> {
    // generated instruction sequence
    pub instrs: InstrSeq<'arena>,
    // does instruction sequence fall through
    is_fallthrough: bool,
    // was label associated with emit operation used
    is_label_used: bool,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum LValOp {
    Set,
    SetOp(EqOp),
    IncDec(IncdecOp),
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
    use crate::*;
    use hash::HashMap;
    use oxidized::{aast_defs::Lid, aast_visitor, aast_visitor::Node, ast, ast_defs};
    use std::marker::PhantomData;

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

    pub(super) fn should_move_local_value<'arena>(
        local: &Local<'arena>,
        aliases: &AliasInfoMap<'_>,
    ) -> bool {
        match local {
            Local::Named(name) => aliases
                .get(name.unsafe_as_str())
                .map_or(true, |alias| alias.has_single_ref()),
            Local::Unnamed(_) => false,
        }
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
        use ast::{Expr, Expr_};
        let Expr(_, _, e) = arg;
        // inout $v
        if let (ParamKind::Pinout(_), Expr_::Lvar(lid)) = (pk, e) {
            let Lid(_, lid) = &**lid;
            if !is_local_this(env, &lid) {
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
        type P = aast_visitor::AstParams<Ctx<'r, 'ast, 'arena>, ()>;

        fn object(&mut self) -> &mut dyn aast_visitor::Visitor<'ast, P = Self::P> {
            self
        }

        fn visit_expr_(
            &mut self,
            c: &mut Ctx<'r, 'ast, 'arena>,
            p: &'ast ast::Expr_,
        ) -> std::result::Result<(), ()> {
            // f(inout $v) or f(&$v)
            if let ast::Expr_::Call(expr) = p {
                let (_, _, args, uarg) = &**expr;
                args.iter()
                    .for_each(|(pk, arg)| handle_arg(&c.env, false, c.i, pk, arg, &mut c.state));
                if let Some(arg) = uarg.as_ref() {
                    handle_arg(&c.env, false, c.i, &ParamKind::Pnormal, arg, &mut c.state)
                }
                Ok(())
            } else {
                p.recurse(c, self.object())?;
                Ok(match p {
                    // lhs op= _
                    ast::Expr_::Binop(expr) => {
                        let (bop, left, _) = &**expr;
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
                        add_use(id, &mut c.state);
                    }
                    _ => {}
                })
            }
        }
    } // impl<'ast, 'a, 'arena> aast_visitor::Visitor<'ast> for Visitor<'a, 'arena>

    // collect lvars on the left hand side of '=' operator
    fn collect_lvars_hs<'r, 'ast, 'arena>(ctx: &mut Ctx<'r, 'ast, 'arena>, expr: &'ast ast::Expr) {
        let ast::Expr(_, _, e) = expr;
        match &*e {
            ast::Expr_::Lvar(lid) => {
                let Lid(_, lid) = &**lid;
                if !is_local_this(&ctx.env, &lid) {
                    add_use(lid.1.as_str(), &mut ctx.state);
                    add_write(lid.1.as_str(), ctx.i, &mut ctx.state);
                }
            }
            ast::Expr_::List(exprs) => exprs.iter().for_each(|expr| collect_lvars_hs(ctx, expr)),
            _ => {}
        }
    }
} //mod inout_locals

pub fn get_type_structure_for_hint<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    tparams: &[&str],
    targ_map: &IndexSet<&str>,
    hint: &aast::Hint,
) -> std::result::Result<InstrSeq<'arena>, hhbc_by_ref_instruction_sequence::Error> {
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
        &hint,
        false,
        false,
    )?;
    let i = Str::from(emit_adata::get_array_identifier(e.alloc, e, &tv));
    Ok(instr::lit_const(e.alloc, InstructLitConst::Dict(i)))
}

pub struct Setrange {
    pub op: SetrangeOp,
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
type InstrSeqWithLocals<'arena> = Vec<(InstrSeq<'arena>, Option<(Local<'arena>, StoredValueKind)>)>;

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

pub fn emit_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expression: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    use aast_defs::Lid;
    use ast::Expr_;
    let alloc = env.arena; // Should this be emitter.alloc?
    let ast::Expr(_, pos, expr) = expression;
    match expr {
        Expr_::Float(_)
        | Expr_::String(_)
        | Expr_::Int(_)
        | Expr_::Null
        | Expr_::False
        | Expr_::True => {
            let v = ast_constant_folder::expr_to_typed_value(alloc, emitter, expression)
                .map_err(|_| unrecoverable("expr_to_typed_value failed"))?;
            Ok(emit_pos_then(alloc, pos, instr::typedvalue(alloc, v)))
        }
        Expr_::EnumClassLabel(label) => {
            // emitting E#A as __Systemlib\create_opaque_value(OpaqueValue::EnumClassLabel, "A")
            let create_opaque_value = "__Systemlib\\create_opaque_value".to_string();
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
            let call_expr = (
                create_opaque_value,
                vec![],
                vec![
                    (ParamKind::Pnormal, enum_class_label_index),
                    (ParamKind::Pnormal, label),
                ],
                None,
            );
            emit_call_expr(emitter, env, pos, None, false, &call_expr)
        }
        Expr_::PrefixedString(e) => emit_expr(emitter, env, &e.1),
        Expr_::Lvar(e) => {
            let Lid(pos, _) = &**e;
            Ok(InstrSeq::gather(
                alloc,
                vec![
                    emit_pos(alloc, pos),
                    emit_local(emitter, env, BareThisOp::Notice, e)?,
                ],
            ))
        }
        Expr_::ClassConst(e) => emit_class_const(emitter, env, pos, &e.0, &e.1),
        Expr_::Unop(e) => emit_unop(emitter, env, pos, e),
        Expr_::Binop(_) => emit_binop(emitter, env, pos, expression),
        Expr_::Pipe(e) => emit_pipe(emitter, env, e),
        Expr_::Is(is_expr) => {
            let (e, h) = &**is_expr;
            let is = emit_is(emitter, env, pos, h)?;
            Ok(InstrSeq::gather(
                alloc,
                vec![emit_expr(emitter, env, e)?, is],
            ))
        }
        Expr_::As(e) => emit_as(emitter, env, pos, e),
        Expr_::Upcast(e) => emit_expr(emitter, env, &e.0),
        Expr_::Cast(e) => emit_cast(emitter, env, pos, &(e.0).1, &e.1),
        Expr_::Eif(e) => emit_conditional_expr(emitter, env, pos, &e.0, &e.1, &e.2),
        Expr_::ArrayGet(e) => {
            let (base_expr, opt_elem_expr) = &**e;
            Ok(emit_array_get(
                emitter,
                env,
                pos,
                None,
                QueryOp::CGet,
                base_expr,
                opt_elem_expr.as_ref(),
                false,
                false,
            )?
            .0)
        }
        Expr_::ObjGet(e) => Ok(emit_obj_get(
            emitter,
            env,
            pos,
            QueryOp::CGet,
            &e.0,
            &e.1,
            &e.2,
            false,
            false,
        )?
        .0),
        Expr_::Call(c) => emit_call_expr(emitter, env, pos, None, false, c),
        Expr_::New(e) => emit_new(emitter, env, pos, e, false),
        Expr_::FunctionPointer(fp) => emit_function_pointer(emitter, env, pos, &fp.0, &fp.1),
        Expr_::Record(e) => emit_record(emitter, env, pos, e),
        Expr_::Darray(e) => Ok(emit_pos_then(
            alloc,
            pos,
            emit_collection(emitter, env, expression, &mk_afkvalues(&e.1), None)?,
        )),
        Expr_::Varray(e) => Ok(emit_pos_then(
            alloc,
            pos,
            emit_collection(emitter, env, expression, &mk_afvalues(&e.1), None)?,
        )),
        Expr_::Collection(e) => emit_named_collection_str(emitter, env, expression, e),
        Expr_::ValCollection(e) => {
            let (kind, _, es) = &**e;
            let fields = mk_afvalues(es);
            let collection_typ = match kind {
                aast_defs::VcKind::Vector => CollectionType::Vector,
                aast_defs::VcKind::ImmVector => CollectionType::ImmVector,
                aast_defs::VcKind::Set => CollectionType::Set,
                aast_defs::VcKind::ImmSet => CollectionType::ImmSet,
                _ => return emit_collection(emitter, env, expression, &fields, None),
            };
            emit_named_collection(emitter, env, pos, expression, &fields, collection_typ)
        }
        Expr_::Pair(e) => {
            let (_, e1, e2) = (**e).to_owned();
            let fields = mk_afvalues(&[e1, e2]);
            emit_named_collection(emitter, env, pos, expression, &fields, CollectionType::Pair)
        }
        Expr_::KeyValCollection(e) => {
            let (kind, _, fields) = &**e;
            let fields = mk_afkvalues(
                &fields
                    .to_owned()
                    .into_iter()
                    .map(|ast::Field(e1, e2)| (e1, e2))
                    .collect::<Vec<_>>()
                    .as_slice(),
            );
            let collection_typ = match kind {
                aast_defs::KvcKind::Map => CollectionType::Map,
                aast_defs::KvcKind::ImmMap => CollectionType::ImmMap,
                _ => return emit_collection(emitter, env, expression, &fields, None),
            };
            emit_named_collection(emitter, env, pos, expression, &fields, collection_typ)
        }
        Expr_::Clone(e) => Ok(emit_pos_then(alloc, pos, emit_clone(emitter, env, e)?)),
        Expr_::Shape(e) => Ok(emit_pos_then(
            alloc,
            pos,
            emit_shape(emitter, env, expression, e)?,
        )),
        Expr_::Await(e) => emit_await(emitter, env, pos, e),
        Expr_::ReadonlyExpr(e) => emit_readonly_expr(emitter, env, pos, e),
        Expr_::Yield(e) => emit_yield(emitter, env, pos, e),
        Expr_::Efun(e) => Ok(emit_pos_then(
            alloc,
            pos,
            emit_lambda(emitter, env, &e.0, &e.1)?,
        )),
        Expr_::ClassGet(e) => {
            // class gets without a readonly expression must be mutable
            emit_class_get(emitter, env, QueryOp::CGet, &e.0, &e.1, ReadonlyOp::Mutable)
        }

        Expr_::String2(es) => emit_string2(emitter, env, pos, es),
        Expr_::Id(e) => Ok(emit_pos_then(alloc, pos, emit_id(emitter, env, e)?)),
        Expr_::Xml(_) => Err(unrecoverable(
            "emit_xhp: syntax should have been converted during rewriting",
        )),
        Expr_::Import(e) => emit_import(emitter, env, pos, &e.0, &e.1),
        Expr_::Omitted => Ok(instr::empty(alloc)),
        Expr_::Lfun(_) => Err(unrecoverable(
            "expected Lfun to be converted to Efun during closure conversion emit_expr",
        )),
        Expr_::List(_) => Err(emit_fatal::raise_fatal_parse(
            pos,
            "list() can only be used as an lvar. Did you mean to use tuple()?",
        )),
        Expr_::Tuple(e) => Ok(emit_pos_then(
            alloc,
            pos,
            emit_collection(emitter, env, expression, &mk_afvalues(&e), None)?,
        )),

        Expr_::This | Expr_::Lplaceholder(_) | Expr_::Dollardollar(_) => {
            unimplemented!("TODO(hrust) Codegen after naming pass on AAST")
        }
        Expr_::ExpressionTree(et) => emit_expr(emitter, env, &et.runtime_expr),
        Expr_::ETSplice(_) => Err(unrecoverable(
            "expression trees: splice should be erased during rewriting",
        )),
        Expr_::FunId(_)
        | Expr_::MethodId(_)
        | Expr_::MethodCaller(_)
        | Expr_::SmethodId(_)
        | Expr_::Hole(_) => {
            unimplemented!("TODO(hrust)")
        }
    }
}

fn emit_exprs_and_error_on_inout<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    exprs: &[(ParamKind, ast::Expr)],
    fn_name: &str,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena; // Should this be emitter.alloc?
    if exprs.is_empty() {
        Ok(instr::empty(alloc))
    } else {
        Ok(InstrSeq::gather(
            alloc,
            exprs
                .iter()
                .map(|(pk, expr)| match pk {
                    ParamKind::Pnormal => emit_expr(e, env, expr),
                    ParamKind::Pinout(p) => Err(emit_fatal::raise_fatal_parse(
                        &Pos::merge(p, expr.pos()).map_err(Error::Unrecoverable)?,
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
    let alloc = env.arena; // Should this be emitter.alloc?
    if exprs.is_empty() {
        Ok(instr::empty(alloc))
    } else {
        Ok(InstrSeq::gather(
            alloc,
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
    use pseudo_consts::*;
    use InstructLitConst::*;

    let alloc = env.arena; // Should this be emitter.alloc?
    let ast_defs::Id(p, s) = id;
    let res = match s.as_str() {
        G__FILE__ => instr::lit_const(alloc, File),
        G__DIR__ => instr::lit_const(alloc, Dir),
        G__METHOD__ => instr::lit_const(alloc, Method),
        G__FUNCTION_CREDENTIAL__ => instr::lit_const(alloc, FuncCred),
        G__CLASS__ => InstrSeq::gather(alloc, vec![instr::self_(alloc), instr::classname(alloc)]),
        G__COMPILER_FRONTEND__ => instr::string(alloc, "hackc"),
        G__LINE__ => instr::int(
            alloc,
            p.info_pos_extended().1.try_into().map_err(|_| {
                emit_fatal::raise_fatal_parse(p, "error converting end of line from usize to isize")
            })?,
        ),
        G__NAMESPACE__ => instr::string(alloc, env.namespace.name.as_ref().map_or("", |s| &s[..])),
        EXIT | DIE => return emit_exit(emitter, env, None),
        _ => {
            // panic!("TODO: uncomment after D19350786 lands")
            // let cid: ConstId = r#const::ConstType::from_ast_name(&s);
            let cid = r#const::ConstType(Str::new_str(alloc, string_utils::strip_global_ns(&s)));
            emit_symbol_refs::add_constant(emitter, cid.clone());
            return Ok(emit_pos_then(alloc, p, instr::lit_const(alloc, CnsE(cid))));
        }
    };
    Ok(res)
}

fn emit_exit<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr_opt: Option<&ast::Expr>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena; // Should this be emitter.alloc?
    Ok(InstrSeq::gather(
        alloc,
        vec![
            expr_opt.map_or_else(|| Ok(instr::int(alloc, 0)), |e| emit_expr(emitter, env, e))?,
            instr::exit(alloc),
        ],
    ))
}

fn emit_yield<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    af: &ast::Afield,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena; // Should this be emitter.alloc?
    Ok(match af {
        ast::Afield::AFvalue(v) => InstrSeq::gather(
            alloc,
            vec![
                emit_expr(e, env, v)?,
                emit_pos(alloc, pos),
                instr::yield_(alloc),
            ],
        ),
        ast::Afield::AFkvalue(k, v) => InstrSeq::gather(
            alloc,
            vec![
                emit_expr(e, env, k)?,
                emit_expr(e, env, v)?,
                emit_pos(alloc, pos),
                instr::yieldk(alloc),
            ],
        ),
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
            ast::Expr_::Binop(x) if x.0.is_dot() => {
                let (v, l) = split_var_lit(&x.2);
                if v.is_empty() {
                    let (var, lit) = split_var_lit(&x.1);
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
                format!("{}[{}]", local_id::get_name(&id), text_of_expr(e_))
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
    emit_symbol_refs::add_include(e, inc.clone());
    let (expr_instrs, import_op_instr) = match flavor {
        ImportFlavor::Include => (emit_expr(e, env, expr)?, instr::incl(alloc)),
        ImportFlavor::Require => (emit_expr(e, env, expr)?, instr::req(alloc)),
        ImportFlavor::IncludeOnce => (emit_expr(e, env, expr)?, instr::inclonce(alloc)),
        ImportFlavor::RequireOnce => {
            match inc.into_doc_root_relative(alloc, e.options().hhvm.include_roots.get()) {
                IncludePath::DocRootRelative(path) => {
                    let expr = ast::Expr(
                        (),
                        pos.clone(),
                        ast::Expr_::String(path.unsafe_as_str().into()),
                    );
                    (emit_expr(e, env, &expr)?, instr::reqdoc(alloc))
                }
                _ => (emit_expr(e, env, expr)?, instr::reqonce(alloc)),
            }
        }
    };
    Ok(InstrSeq::gather(
        alloc,
        vec![expr_instrs, emit_pos(alloc, pos), import_op_instr],
    ))
}

fn emit_string2<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    es: &[ast::Expr],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena; // Should this be emitter.alloc?
    if es.is_empty() {
        Err(unrecoverable("String2 with zero araguments is impossible"))
    } else if es.len() == 1 {
        Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_expr(e, env, &es[0])?,
                emit_pos(alloc, pos),
                instr::cast_string(alloc),
            ],
        ))
    } else {
        Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_two_exprs(e, env, &es[0].1, &es[0], &es[1])?,
                emit_pos(alloc, pos),
                instr::concat(alloc),
                InstrSeq::gather(
                    alloc,
                    (&es[2..])
                        .iter()
                        .map(|expr| {
                            Ok(InstrSeq::gather(
                                alloc,
                                vec![
                                    emit_expr(e, env, expr)?,
                                    emit_pos(alloc, pos),
                                    instr::concat(alloc),
                                ],
                            ))
                        })
                        .collect::<Result<_>>()?,
                ),
            ],
        ))
    }
}

fn emit_clone<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena; // Should this be emitter.alloc?
    Ok(InstrSeq::gather(
        alloc,
        vec![emit_expr(e, env, expr)?, instr::clone(alloc)],
    ))
}

fn emit_lambda<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    fndef: &ast::Fun_,
    ids: &[aast_defs::Lid],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena; // etc.
    // Closure conversion puts the class number used for CreateCl in the "name"
    // of the function definition
    let fndef_name = &(fndef.name).1;
    let cls_num = fndef_name
        .parse::<isize>()
        .map_err(|err| Unrecoverable(err.to_string()))?;
    let explicit_use = e.emit_global_state().explicit_use_set.contains(fndef_name);
    let is_in_lambda = env.scope.is_in_lambda();
    Ok(InstrSeq::gather(
        alloc,
        vec![
            InstrSeq::gather(
                alloc,
                ids.iter()
                    .map(|ast::Lid(pos, id)| {
                        match string_utils::reified::is_captured_generic(local_id::get_name(id)) {
                            Some((is_fun, i)) => {
                                if is_in_lambda {
                                    Ok(instr::cgetl(
                                        alloc,
                                        Local::Named(Str::new_str(
                                            alloc,
                                            string_utils::reified::reified_generic_captured_name(
                                                is_fun, i as usize,
                                            )
                                            .as_str(),
                                        )),
                                    ))
                                } else {
                                    emit_reified_generic_instrs(
                                        alloc,
                                        &Pos::make_none(),
                                        is_fun,
                                        i as usize,
                                    )
                                }
                            }
                            None => Ok({
                                let lid = get_local(e, env, pos, local_id::get_name(id))?;
                                if explicit_use {
                                    instr::cgetl(alloc, lid)
                                } else {
                                    instr::cugetl(alloc, lid)
                                }
                            }),
                        }
                    })
                    .collect::<Result<Vec<_>>>()?,
            ),
            instr::createcl(alloc, ids.len(), cls_num),
        ],
    ))
}

pub fn emit_await<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let ast::Expr(_, _, e) = expr;
    let alloc = env.arena;
    let cant_inline_gen_functions = !emitter
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION);
    match e.as_call() {
        Some((ast::Expr(_, _, ast::Expr_::Id(id)), _, args, None))
            if (cant_inline_gen_functions
                && args.len() == 1
                && string_utils::strip_global_ns(&(*id.1)) == "gena") =>
        {
            inline_gena_call(emitter, env, expect_normal_paramkind(&args[0])?)
        }
        _ => {
            let after_await = emitter.label_gen_mut().next_regular();
            let instrs = match e {
                ast::Expr_::Call(c) => {
                    emit_call_expr(emitter, env, pos, Some(after_await.clone()), false, &*c)?
                }
                _ => emit_expr(emitter, env, expr)?,
            };
            Ok(InstrSeq::gather(
                alloc,
                vec![
                    instrs,
                    emit_pos(alloc, pos),
                    instr::dup(alloc),
                    instr::istypec(alloc, IstypeOp::OpNull),
                    instr::jmpnz(alloc, after_await.clone()),
                    instr::await_(alloc),
                    instr::label(alloc, after_await),
                ],
            ))
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
        let before = InstrSeq::gather(
            alloc,
            vec![
                load_arr,
                instr::cast_dict(alloc),
                instr::popl(alloc, arr_local),
            ],
        );

        let inner = InstrSeq::gather(
            alloc,
            vec![
                instr::nulluninit(alloc),
                instr::nulluninit(alloc),
                instr::cgetl(alloc, arr_local),
                instr::fcallclsmethodd(
                    alloc,
                    FcallArgs::new(
                        FcallFlags::default(),
                        1,
                        Slice::empty(),
                        Slice::empty(),
                        Some(async_eager_label),
                        1,
                        None,
                    ),
                    method::from_raw_string(alloc, "fromDict"),
                    class::from_raw_string(alloc, "HH\\AwaitAllWaitHandle"),
                ),
                instr::await_(alloc),
                instr::label(alloc, async_eager_label),
                instr::popc(alloc),
                emit_iter(
                    e,
                    instr::cgetl(alloc, arr_local),
                    |alloc, val_local, key_local| {
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::cgetl(alloc, val_local),
                                instr::whresult(alloc),
                                instr::basel(
                                    alloc,
                                    arr_local,
                                    MemberOpMode::Define,
                                    ReadonlyOp::Any, // TODO, handle await assignment statements correctly
                                ),
                                instr::setm(alloc, 0, MemberKey::EL(key_local, ReadonlyOp::Any)),
                                instr::popc(alloc),
                            ],
                        )
                    },
                )?,
            ],
        );

        let after = instr::pushl(alloc, arr_local);

        Ok((before, inner, after))
    })
}

fn emit_iter<
    'arena,
    'decl,
    F: FnOnce(&'arena bumpalo::Bump, Local<'arena>, Local<'arena>) -> InstrSeq<'arena>,
>(
    e: &mut Emitter<'arena, 'decl>,
    collection: InstrSeq<'arena>,
    f: F,
) -> Result<InstrSeq<'arena>> {
    scope::with_unnamed_locals_and_iterators(e, |e| {
        let alloc = e.alloc;
        let iter_id = e.iterator_mut().get();
        let val_id = e.local_gen_mut().get_unnamed();
        let key_id = e.local_gen_mut().get_unnamed();
        let loop_end = e.label_gen_mut().next_regular();
        let loop_next = e.label_gen_mut().next_regular();
        let iter_args = IterArgs {
            iter_id,
            key_id: Just(key_id),
            val_id,
        };
        let iter_init = InstrSeq::gather(
            alloc,
            vec![
                collection,
                instr::iterinit(alloc, iter_args.clone(), loop_end),
            ],
        );
        let iterate = InstrSeq::gather(
            alloc,
            vec![
                instr::label(alloc, loop_next),
                f(alloc, val_id, key_id),
                instr::iternext(alloc, iter_args, loop_next),
            ],
        );
        let iter_done = InstrSeq::gather(
            alloc,
            vec![
                instr::unsetl(alloc, val_id),
                instr::unsetl(alloc, key_id),
                instr::label(alloc, loop_end),
            ],
        );
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
                if is_reified_tparam(env, true, &id.1).is_some()
                    || is_reified_tparam(env, false, &id.1).is_some()
                {
                    return Err(emit_fatal::raise_fatal_parse(
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
            Ok((
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
        &ast::Expr((), pos.clone(), ast::Expr_::mk_darray(None, fl)),
    )
}

fn emit_vec_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match ast_constant_folder::vec_to_typed_value(alloc, e, fields) {
        Ok(tv) => emit_static_collection(env, None, pos, tv),
        Err(_) => emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec),
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
    let alloc = env.arena;
    let emit_vector_like = |e: &mut Emitter<'arena, 'decl>, collection_type| {
        Ok(if fields.is_empty() {
            emit_pos_then(alloc, pos, instr::newcol(alloc, collection_type))
        } else {
            InstrSeq::gather(
                alloc,
                vec![
                    emit_vec_collection(e, env, pos, fields)?,
                    instr::colfromarray(alloc, collection_type),
                ],
            )
        })
    };
    let emit_map_or_set = |e: &mut Emitter<'arena, 'decl>, collection_type| {
        if fields.is_empty() {
            Ok(emit_pos_then(
                alloc,
                pos,
                instr::newcol(alloc, collection_type),
            ))
        } else {
            emit_collection(e, env, expr, fields, Some(collection_type))
        }
    };
    use CollectionType as C;
    match collection_type {
        C::Dict | C::Vec | C::Keyset => {
            let instr = emit_collection(e, env, expr, fields, None)?;
            Ok(emit_pos_then(alloc, pos, instr))
        }
        C::Vector | C::ImmVector => emit_vector_like(e, collection_type),
        C::Map | C::ImmMap | C::Set | C::ImmSet => emit_map_or_set(e, collection_type),
        C::Pair => Ok(InstrSeq::gather(
            alloc,
            vec![
                InstrSeq::gather(
                    alloc,
                    fields
                        .iter()
                        .map(|f| match f {
                            ast::Afield::AFvalue(v) => emit_expr(e, env, v),
                            _ => Err(unrecoverable("impossible Pair argument")),
                        })
                        .collect::<Result<_>>()?,
                ),
                instr::new_pair(alloc),
            ],
        )),
        _ => Err(unrecoverable("Unexpected named collection type")),
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
    use CollectionType::*;
    let ctype = match name {
        "dict" => Dict,
        "vec" => Vec,
        "keyset" => Keyset,
        "Vector" => Vector,
        "ImmVector" => ImmVector,
        "Map" => Map,
        "ImmMap" => ImmMap,
        "Set" => Set,
        "ImmSet" => ImmSet,
        "Pair" => Pair,
        _ => {
            return Err(unrecoverable(format!(
                "collection: {} does not exist",
                name
            )));
        }
    };
    emit_named_collection(e, env, pos, expr, fields, ctype)
}

fn mk_afkvalues(es: &[(ast::Expr, ast::Expr)]) -> Vec<ast::Afield> {
    es.to_owned()
        .into_iter()
        .map(|(e1, e2)| ast::Afield::mk_afkvalue(e1, e2))
        .collect()
}

fn mk_afvalues(es: &[ast::Expr]) -> Vec<ast::Afield> {
    es.to_owned()
        .into_iter()
        .map(ast::Afield::mk_afvalue)
        .collect()
}

fn emit_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    fields: &[ast::Afield],
    transform_to_collection: Option<CollectionType>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let pos = &expr.1;
    match ast_constant_folder::expr_to_typed_value_(
        alloc, e, expr, true,  /*allow_map*/
        false, /*force_class_const*/
    ) {
        Ok(tv) => emit_static_collection(env, transform_to_collection, pos, tv),
        Err(_) => emit_dynamic_collection(e, env, expr, fields),
    }
}

fn emit_static_collection<'a, 'arena, 'decl>(
    env: &Env<'a, 'arena>,
    transform_to_collection: Option<CollectionType>,
    pos: &Pos,
    tv: TypedValue<'arena>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let transform_instr = match transform_to_collection {
        Some(collection_type) => instr::colfromarray(alloc, collection_type),
        _ => instr::empty(alloc),
    };
    Ok(InstrSeq::gather(
        alloc,
        vec![
            emit_pos(alloc, pos),
            instr::typedvalue(alloc, tv),
            transform_instr,
        ],
    ))
}

fn expr_and_new<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    instr_to_add_new: InstrSeq<'arena>,
    instr_to_add: InstrSeq<'arena>,
    field: &ast::Afield,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match field {
        ast::Afield::AFvalue(v) => Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_expr(e, env, v)?,
                emit_pos(alloc, pos),
                instr_to_add_new,
            ],
        )),
        ast::Afield::AFkvalue(k, v) => Ok(InstrSeq::gather(
            alloc,
            vec![emit_two_exprs(e, env, &k.1, k, v)?, instr_to_add],
        )),
    }
}

fn emit_keyvalue_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
    ctype: CollectionType,
    constructor: InstructLitConst<'arena>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let (transform_instr, add_elem_instr) = match ctype {
        CollectionType::Dict | CollectionType::Array => {
            (instr::empty(alloc), instr::add_new_elemc(alloc))
        }
        _ => (
            instr::colfromarray(alloc, ctype),
            InstrSeq::gather(alloc, vec![instr::dup(alloc), instr::add_elemc(alloc)]),
        ),
    };
    let emitted_pos = emit_pos(alloc, pos);
    Ok(InstrSeq::gather(
        alloc,
        vec![
            InstrSeq::<'arena>::clone(alloc, &emitted_pos),
            instr::lit_const(alloc, constructor),
            fields
                .iter()
                .map(|f| {
                    expr_and_new(
                        e,
                        env,
                        pos,
                        InstrSeq::<'arena>::clone(alloc, &add_elem_instr),
                        instr::add_elemc(alloc),
                        f,
                    )
                })
                .collect::<Result<_>>()
                .map(|s| InstrSeq::gather(alloc, s))?,
            emitted_pos,
            transform_instr,
        ],
    ))
}

fn non_numeric(s: &str) -> bool {
    // Note(hrust): OCaml Int64.of_string and float_of_string ignore underscores
    let s = s.replace("_", "");
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
    fn int_from_str(s: &str) -> std::result::Result<i64, ()> {
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
            i64::from_str(&s)
        })
        .map_err(|_| ())
    }
    fn float_from_str_radix(s: &str, radix: u32) -> std::result::Result<f64, ()> {
        let i = i64::from_str_radix(&s.replace(".", ""), radix).map_err(|_| ())?;
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
    fn float_from_str(s: &str) -> std::result::Result<f64, ()> {
        // Note(hrust): OCaml float_of_string ignores leading whitespace, reads decimal and hexadecimal
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

fn is_struct_init<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    fields: &[ast::Afield],
    allow_numerics: bool,
) -> Result<bool> {
    let alloc = env.arena;
    let mut are_all_keys_non_numeric_strings = true;
    let mut uniq_keys = HashSet::<bstr::BString>::default();
    for f in fields.iter() {
        if let ast::Afield::AFkvalue(key, _) = f {
            // TODO(hrust): if key is String, don't clone and call fold_expr
            let mut key = key.clone();
            ast_constant_folder::fold_expr(&mut key, alloc, e)
                .map_err(|e| unrecoverable(format!("{}", e)))?;
            if let ast::Expr(_, _, ast::Expr_::String(s)) = key {
                are_all_keys_non_numeric_strings = are_all_keys_non_numeric_strings
                    && non_numeric(
                        // FIXME: This is not safe--string literals are binary strings.
                        // There's no guarantee that they're valid UTF-8.
                        unsafe { std::str::from_utf8_unchecked(s.as_slice()) },
                    );
                uniq_keys.insert(s);
            } else {
                are_all_keys_non_numeric_strings = false;
            }
            continue;
        }
        are_all_keys_non_numeric_strings = false;
    }
    let num_keys = fields.len();
    let limit = *(e.options().max_array_elem_size_on_the_stack.get()) as usize;
    Ok((allow_numerics || are_all_keys_non_numeric_strings)
        && uniq_keys.len() == num_keys
        && num_keys <= limit
        && num_keys != 0)
}

fn emit_struct_array<
    'a,
    'arena,
    'decl,
    C: FnOnce(
        &'arena bumpalo::Bump,
        &mut Emitter<'arena, 'decl>,
        &'arena [&'arena str],
    ) -> Result<InstrSeq<'arena>>,
>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
    ctor: C,
) -> Result<InstrSeq<'arena>> {
    use ast::{Expr as E, Expr_ as E_};
    let alloc = env.arena;
    let (keys, value_instrs): (Vec<String>, _) = fields
        .iter()
        .map(|f| match f {
            ast::Afield::AFkvalue(k, v) => match k {
                E(_, _, E_::String(s)) => Ok((
                    // FIXME: This is not safe--string literals are binary strings.
                    // There's no guarantee that they're valid UTF-8.
                    unsafe { String::from_utf8_unchecked(s.clone().into()) },
                    emit_expr(e, env, v)?,
                )),
                _ => {
                    let mut k = k.clone();
                    ast_constant_folder::fold_expr(&mut k, alloc, e)
                        .map_err(|e| unrecoverable(format!("{}", e)))?;
                    match k {
                        E(_, _, E_::String(s)) => Ok((
                            // FIXME: This is not safe--string literals are binary strings.
                            // There's no guarantee that they're valid UTF-8.
                            unsafe { String::from_utf8_unchecked(s.into()) },
                            emit_expr(e, env, v)?,
                        )),
                        _ => Err(unrecoverable("Key must be a string")),
                    }
                }
            },
            _ => Err(unrecoverable("impossible")),
        })
        .collect::<Result<Vec<(String, InstrSeq<'arena>)>>>()?
        .into_iter()
        .unzip();
    let keys_ = bumpalo::collections::Vec::from_iter_in(
        keys.into_iter()
            .map(|x| bumpalo::collections::String::from_str_in(x.as_str(), alloc).into_bump_str()),
        alloc,
    )
    .into_bump_slice();
    Ok(InstrSeq::gather(
        alloc,
        vec![
            InstrSeq::gather(alloc, value_instrs),
            emit_pos(alloc, pos),
            ctor(alloc, e, keys_)?,
        ],
    ))
}

fn emit_dynamic_collection<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    fields: &[ast::Afield],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let pos = &expr.1;
    let count = fields.len();
    let emit_dict = |e: &mut Emitter<'arena, 'decl>| {
        if is_struct_init(e, env, fields, true)? {
            emit_struct_array(e, env, pos, fields, |alloc, _, x| {
                Ok(instr::newstructdict(alloc, x))
            })
        } else {
            let ctor = InstructLitConst::NewDictArray(count as isize);
            emit_keyvalue_collection(e, env, pos, fields, CollectionType::Dict, ctor)
        }
    };
    let emit_collection_helper = |e: &mut Emitter<'arena, 'decl>, ctype| {
        if is_struct_init(e, env, fields, true)? {
            Ok(InstrSeq::gather(
                alloc,
                vec![
                    emit_struct_array(e, env, pos, fields, |alloc, _, x| {
                        Ok(instr::newstructdict(alloc, x))
                    })?,
                    emit_pos(alloc, pos),
                    instr::colfromarray(alloc, ctype),
                ],
            ))
        } else {
            let ctor = InstructLitConst::NewDictArray(count as isize);
            emit_keyvalue_collection(e, env, pos, fields, ctype, ctor)
        }
    };
    use ast::Expr_ as E_;
    match &expr.2 {
        E_::ValCollection(v) if v.0 == ast::VcKind::Vec => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec)
        }
        E_::Collection(v) if (v.0).1 == "vec" => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec)
        }
        E_::Tuple(_) => emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec),
        E_::ValCollection(v) if v.0 == ast::VcKind::Keyset => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewKeysetArray)
        }
        E_::Collection(v) if (v.0).1 == "keyset" => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewKeysetArray)
        }
        E_::Collection(v) if (v.0).1 == "dict" => emit_dict(e),
        E_::KeyValCollection(v) if v.0 == ast::KvcKind::Dict => emit_dict(e),
        E_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "Set" => {
            emit_collection_helper(e, CollectionType::Set)
        }
        E_::ValCollection(v) if v.0 == ast::VcKind::Set => {
            emit_collection_helper(e, CollectionType::Set)
        }
        E_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "ImmSet" => {
            emit_collection_helper(e, CollectionType::ImmSet)
        }
        E_::ValCollection(v) if v.0 == ast::VcKind::ImmSet => {
            emit_collection_helper(e, CollectionType::ImmSet)
        }
        E_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "Map" => {
            emit_collection_helper(e, CollectionType::Map)
        }
        E_::KeyValCollection(v) if v.0 == ast::KvcKind::Map => {
            emit_collection_helper(e, CollectionType::Map)
        }
        E_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "ImmMap" => {
            emit_collection_helper(e, CollectionType::ImmMap)
        }
        E_::KeyValCollection(v) if v.0 == ast::KvcKind::ImmMap => {
            emit_collection_helper(e, CollectionType::ImmMap)
        }
        E_::Varray(_) => {
            let instrs = emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec);
            Ok(instrs?)
        }
        E_::Darray(_) => {
            if is_struct_init(e, env, fields, false /* allow_numerics */)? {
                let instrs = emit_struct_array(e, env, pos, fields, |alloc, _, arg| {
                    let instr = instr::newstructdict(alloc, arg);
                    Ok(emit_pos_then(alloc, pos, instr))
                });
                Ok(instrs?)
            } else {
                let constr = InstructLitConst::NewDictArray(count as isize);
                let instrs =
                    emit_keyvalue_collection(e, env, pos, fields, CollectionType::Array, constr);
                Ok(instrs?)
            }
        }
        _ => Err(unrecoverable("plain PHP arrays cannot be constructed")),
    }
}

fn emit_value_only_collection<'a, 'arena, 'decl, F: FnOnce(isize) -> InstructLitConst<'arena>>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    fields: &[ast::Afield],
    constructor: F,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let limit = *(e.options().max_array_elem_size_on_the_stack.get()) as usize;
    let inline = |
        alloc: &'arena bumpalo::Bump,
        e: &mut Emitter<'arena, 'decl>,
        exprs: &[ast::Afield],
    | -> Result<InstrSeq<'arena>> {
        let mut instrs = vec![];
        for expr in exprs.iter() {
            instrs.push(emit_expr(e, env, expr.value())?)
        }

        Ok(InstrSeq::gather(
            alloc,
            vec![
                InstrSeq::gather(alloc, instrs),
                emit_pos(alloc, pos),
                instr::lit_const(alloc, constructor(exprs.len() as isize)),
            ],
        ))
    };
    let outofline = |
        alloc: &'arena bumpalo::Bump,
        e: &mut Emitter<'arena, 'decl>,
        exprs: &[ast::Afield],
    | -> Result<InstrSeq<'arena>> {
        let mut instrs = vec![];
        for expr in exprs.iter() {
            instrs.push(emit_expr(e, env, expr.value())?);
            instrs.push(instr::add_new_elemc(alloc));
        }
        Ok(InstrSeq::gather(alloc, instrs))
    };
    let (x1, x2) = fields.split_at(std::cmp::min(fields.len(), limit));
    Ok(match (x1, x2) {
        ([], []) => instr::empty(alloc),
        (_, []) => inline(alloc, e, x1)?,
        _ => {
            let outofline_instrs = outofline(alloc, e, x2)?;
            let inline_instrs = inline(alloc, e, x1)?;
            InstrSeq::gather(alloc, vec![inline_instrs, outofline_instrs])
        }
    })
}

fn emit_record<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    (cid, es): &(ast::Sid, Vec<(ast::Expr, ast::Expr)>),
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let es = mk_afkvalues(es);
    let id = class::ClassType::from_ast_name_and_mangle(alloc, &cid.1);
    emit_symbol_refs::add_class(e, id);
    emit_struct_array(e, env, pos, &es, |alloc, _, keys| {
        Ok(instr::new_record(alloc, id, keys))
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
        return Err(emit_fatal::raise_fatal_parse(
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
            QueryOp::Isset,
            base_expr,
            opt_elem_expr.as_ref(),
            false,
            false,
        )?
        .0);
    }
    if let Some((cid, id, _)) = expr.2.as_class_get() {
        return emit_class_get(e, env, QueryOp::Isset, cid, id, ReadonlyOp::Any);
    }
    if let Some((expr_, prop, nullflavor, _)) = expr.2.as_obj_get() {
        return Ok(emit_obj_get(
            e,
            env,
            pos,
            QueryOp::Isset,
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
        return Ok(if superglobals::is_any_global(&name) {
            InstrSeq::gather(
                alloc,
                vec![
                    emit_pos(alloc, outer_pos),
                    instr::string(alloc, string_utils::locals::strip_dollar(&name)),
                    emit_pos(alloc, outer_pos),
                    instr::issetg(alloc),
                ],
            )
        } else if is_local_this(env, &lid.1)
            && !env.flags.contains(hhbc_by_ref_env::Flags::NEEDS_LOCAL_THIS)
        {
            InstrSeq::gather(
                alloc,
                vec![
                    emit_pos(alloc, outer_pos),
                    emit_local(e, env, BareThisOp::NoNotice, lid)?,
                    emit_pos(alloc, outer_pos),
                    instr::istypec(alloc, IstypeOp::OpNull),
                    instr::not(alloc),
                ],
            )
        } else {
            emit_pos_then(
                alloc,
                outer_pos,
                instr::issetl(alloc, get_local(e, env, &lid.0, name)?),
            )
        });
    }
    Ok(InstrSeq::gather(
        alloc,
        vec![
            emit_expr(e, env, expr)?,
            instr::istypec(alloc, IstypeOp::OpNull),
            instr::not(alloc),
        ],
    ))
}

fn emit_call_isset_exprs<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    exprs: &[(ParamKind, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match exprs {
        [] => Err(emit_fatal::raise_fatal_parse(
            pos,
            "Cannot use isset() without any arguments",
        )),
        [(pk, expr)] => emit_call_isset_expr(e, env, pos, pk, expr),
        _ => {
            let its_done = e.label_gen_mut().next_regular();
            Ok(InstrSeq::gather(
                alloc,
                vec![
                    InstrSeq::gather(
                        alloc,
                        exprs
                            .iter()
                            .enumerate()
                            .map(|(i, (pk, expr))| {
                                Ok(InstrSeq::gather(
                                    alloc,
                                    vec![
                                        emit_call_isset_expr(e, env, pos, pk, expr)?,
                                        if i < exprs.len() - 1 {
                                            InstrSeq::gather(
                                                alloc,
                                                vec![
                                                    instr::dup(alloc),
                                                    instr::jmpz(alloc, its_done),
                                                    instr::popc(alloc),
                                                ],
                                            )
                                        } else {
                                            instr::empty(alloc)
                                        },
                                    ],
                                ))
                            })
                            .collect::<Result<Vec<_>>>()?,
                    ),
                    instr::label(alloc, its_done),
                ],
            ))
        }
    }
}

fn emit_tag_provenance_here<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    es: &[(ParamKind, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let pop = if es.len() == 1 {
        instr::empty(alloc)
    } else {
        instr::popc(alloc)
    };
    Ok(InstrSeq::gather(
        alloc,
        vec![
            emit_exprs_and_error_on_inout(e, env, es, "HH\\tag_provenance_here")?,
            emit_pos(alloc, pos),
            pop,
        ],
    ))
}

fn emit_array_mark_legacy<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    es: &[(ParamKind, ast::Expr)],
    legacy: bool,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let default = if es.len() == 1 {
        instr::false_(alloc)
    } else {
        instr::empty(alloc)
    };
    let mark = if legacy {
        instr::instr(alloc, Instruct::IMisc(InstructMisc::ArrayMarkLegacy))
    } else {
        instr::instr(alloc, Instruct::IMisc(InstructMisc::ArrayUnmarkLegacy))
    };
    Ok(InstrSeq::gather(
        alloc,
        vec![
            emit_exprs_and_error_on_inout(e, env, es, "HH\\array_mark_legacy")?,
            emit_pos(alloc, pos),
            default,
            mark,
        ],
    ))
}

fn emit_idx<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    es: &[(ParamKind, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let default = if es.len() == 2 {
        instr::null(alloc)
    } else {
        instr::empty(alloc)
    };
    Ok(InstrSeq::gather(
        alloc,
        vec![
            emit_exprs_and_error_on_inout(e, env, es, "idx")?,
            emit_pos(alloc, pos),
            default,
            instr::idx(alloc),
        ],
    ))
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
        let fid = function::FunctionType::<'arena>::from_ast_name(alloc, s);
        emit_symbol_refs::add_function(e, fid);
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
            let fq = function::FunctionType::<'arena>::from_ast_name(alloc, id);
            let lower_fq_name = fq.to_raw_string();
            emit_special_function(e, env, pos, args, uarg, lower_fq_name)
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
    fcall_args: FcallArgs<'arena>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    scope::with_unnamed_locals(e, |em| {
        let FcallArgs(_, _, num_ret, _, _, _, _) = &fcall_args;
        let num_uninit = num_ret - 1;
        let (lhs, fcall) = emit_call_lhs_and_fcall(em, env, expr, fcall_args, targs, None)?;
        let (args, inout_setters) = emit_args_inout_setters(em, env, args)?;
        let uargs = match uarg {
            Some(uarg) => emit_expr(em, env, uarg)?,
            None => instr::empty(alloc),
        };
        Ok((
            instr::empty(alloc),
            InstrSeq::gather(
                alloc,
                vec![
                    InstrSeq::gather(
                        alloc,
                        iter::repeat_with(|| instr::nulluninit(alloc))
                            .take(num_uninit)
                            .collect::<Vec<_>>(),
                    ),
                    lhs,
                    args,
                    uargs,
                    emit_pos(alloc, pos),
                    fcall,
                    inout_setters,
                ],
            ),
            instr::empty(alloc),
        ))
    })
}

pub fn emit_reified_targs<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    targs: &[&ast::Hint],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let current_fun_tparams = env.scope.get_fun_tparams();
    let current_cls_tparams = env.scope.get_class_tparams();
    let is_in_lambda = env.scope.is_in_lambda();
    let is_soft =
        |ual: &Vec<ast::UserAttribute>| ual.iter().any(|ua| user_attributes::is_soft(&ua.name.1));
    let same_as_targs = |tparams: &[ast::Tparam]| {
        tparams.len() == targs.len()
            && tparams.iter().zip(targs).all(|(tp, ta)| {
                ta.1.as_happly().map_or(false, |(id, hs)| {
                    id.1 == tp.name.1
                        && hs.is_empty()
                        && !is_soft(&tp.user_attributes)
                        && tp.reified.is_reified()
                })
            })
    };
    Ok(if !is_in_lambda && same_as_targs(&current_fun_tparams) {
        instr::cgetl(
            alloc,
            Local::Named(Str::new_str(
                alloc,
                string_utils::reified::GENERICS_LOCAL_NAME,
            )),
        )
    } else if !is_in_lambda && same_as_targs(&current_cls_tparams[..]) {
        InstrSeq::gather(
            alloc,
            vec![
                instr::checkthis(alloc),
                instr::baseh(alloc),
                instr::querym(
                    alloc,
                    0,
                    QueryOp::CGet,
                    MemberKey::PT(
                        prop::from_raw_string(alloc, string_utils::reified::PROP_NAME),
                        ReadonlyOp::Any,
                    ),
                ),
            ],
        )
    } else {
        InstrSeq::gather(
            alloc,
            vec![
                InstrSeq::gather(
                    alloc,
                    targs
                        .iter()
                        .map(|h| Ok(emit_reified_arg(e, env, pos, false, h)?.0))
                        .collect::<Result<Vec<_>>>()?,
                ),
                instr::new_vec_array(alloc, targs.len() as isize),
            ],
        )
    })
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

fn from_ast_null_flavor(nullflavor: ast::OgNullFlavor) -> ObjNullFlavor {
    match nullflavor {
        ast::OgNullFlavor::OGNullsafe => ObjNullFlavor::NullSafe,
        ast::OgNullFlavor::OGNullthrows => ObjNullFlavor::NullThrows,
    }
}

fn emit_object_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match &expr.2 {
        ast::Expr_::Lvar(x) if is_local_this(env, &x.1) => Ok(instr::this(alloc)),
        _ => emit_expr(e, env, expr),
    }
}

fn emit_call_lhs_and_fcall<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    mut fcall_args: FcallArgs<'arena>,
    targs: &[ast::Targ],
    caller_readonly_opt: Option<&Pos>,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    let ast::Expr(_, pos, expr_) = expr;
    use ast::{Expr as E, Expr_ as E_};
    let alloc = env.arena;
    let emit_generics =
        |e: &mut Emitter<'arena, 'decl>, env, fcall_args: &mut FcallArgs<'arena>| {
            let does_not_have_non_tparam_generics = !has_non_tparam_generics_targs(env, targs);
            if does_not_have_non_tparam_generics {
                Ok(instr::empty(alloc))
            } else {
                fcall_args.0 |= FcallFlags::HAS_GENERICS;
                emit_reified_targs(
                    e,
                    env,
                    pos,
                    targs
                        .iter()
                        .map(|targ| &targ.1)
                        .collect::<Vec<_>>()
                        .as_slice(),
                )
            }
        };

    let emit_fcall_func = |
        e: &mut Emitter<'arena, 'decl>,
        env,
        expr: &ast::Expr,
        fcall_args: FcallArgs<'arena>,
        caller_readonly_opt: Option<&Pos>,
    | -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
        let tmp = e.local_gen_mut().get_unnamed();
        // if the original expression was wrapped in readonly, emit a readonly expression here
        let res = if let Some(p) = caller_readonly_opt {
            emit_readonly_expr(e, env, p, expr)?
        } else {
            emit_expr(e, env, expr)?
        };
        Ok((
            InstrSeq::gather(
                alloc,
                vec![
                    instr::nulluninit(alloc),
                    instr::nulluninit(alloc),
                    res,
                    instr::popl(alloc, tmp),
                ],
            ),
            InstrSeq::gather(
                alloc,
                vec![
                    instr::pushl(alloc, tmp),
                    instr::fcallfunc(alloc, fcall_args),
                ],
            ),
        ))
    };

    match expr_ {
        E_::ReadonlyExpr(r) => {
            // If calling a Readonly expression, first recurse inside to
            // handle ObjGet and ClassGet prop call cases. Keep track of the position of the
            // outer readonly expression for use later.
            // TODO: use the fact that this is a readonly call in HHVM enforcement
            emit_call_lhs_and_fcall(e, env, r, fcall_args, targs, Some(pos))
        }
        E_::ObjGet(o) if o.as_ref().3 == ast::PropOrMethod::IsMethod => {
            // Case $x->foo(...).
            // TODO: utilize caller_readonly_opt here for method calls
            let emit_id = |
                e: &mut Emitter<'arena, 'decl>,
                obj,
                id,
                null_flavor: &ast::OgNullFlavor,
                mut fcall_args,
            | {
                let name =
                    method::MethodType(Str::new_str(alloc, string_utils::strip_global_ns(id)));
                let obj = emit_object_expr(e, env, obj)?;
                let generics = emit_generics(e, env, &mut fcall_args)?;
                let null_flavor = from_ast_null_flavor(*null_flavor);
                Ok((
                    InstrSeq::gather(alloc, vec![obj, instr::nulluninit(alloc)]),
                    InstrSeq::gather(
                        alloc,
                        vec![
                            generics,
                            instr::fcallobjmethodd(alloc, fcall_args, name, null_flavor),
                        ],
                    ),
                ))
            };
            match o.as_ref() {
                (obj, E(_, _, E_::String(id)), null_flavor, _) => {
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
                (E(_, pos, E_::New(new_exp)), E(_, _, E_::Id(id)), null_flavor, _)
                    if fcall_args.1 == 0 =>
                {
                    let cexpr =
                        ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, &new_exp.0);
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
                                    let fcall_args = FcallArgs::new(
                                        FcallFlags::default(),
                                        1,
                                        Slice::empty(),
                                        Slice::empty(),
                                        None,
                                        1,
                                        None,
                                    );
                                    let newobj_instrs = emit_new(e, env, pos, &new_exp, true);
                                    Ok((
                                        InstrSeq::gather(
                                            alloc,
                                            vec![
                                                instr::nulluninit(alloc),
                                                instr::nulluninit(alloc),
                                                newobj_instrs?,
                                            ],
                                        ),
                                        InstrSeq::gather(
                                            alloc,
                                            vec![instr::fcallfuncd(
                                                alloc,
                                                fcall_args,
                                                function::FunctionType::<'arena>::from_ast_name(
                                                    alloc, fid,
                                                ),
                                            )],
                                        ),
                                    ))
                                }
                            }
                        }
                        _ => emit_id(e, &o.as_ref().0, &id.1, null_flavor, fcall_args),
                    }
                }
                (obj, E(_, _, E_::Id(id)), null_flavor, _) => {
                    emit_id(e, obj, &id.1, null_flavor, fcall_args)
                }
                (obj, method_expr, null_flavor, _) => {
                    let obj = emit_object_expr(e, env, obj)?;
                    let tmp = e.local_gen_mut().get_unnamed();
                    let null_flavor = from_ast_null_flavor(*null_flavor);
                    Ok((
                        InstrSeq::gather(
                            alloc,
                            vec![
                                obj,
                                instr::nulluninit(alloc),
                                emit_expr(e, env, method_expr)?,
                                instr::popl(alloc, tmp),
                            ],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::pushl(alloc, tmp),
                                instr::fcallobjmethod(alloc, fcall_args, null_flavor),
                            ],
                        ),
                    ))
                }
            }
        }
        E_::ClassConst(cls_const) => {
            let (cid, (_, id)) = &**cls_const;
            let mut cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
            if let ClassExpr::Id(ast_defs::Id(_, name)) = &cexpr {
                if let Some(reified_var_cexpr) = get_reified_var_cexpr(env, pos, &name)? {
                    cexpr = reified_var_cexpr;
                }
            }
            let method_id =
                method::MethodType(Str::new_str(alloc, string_utils::strip_global_ns(&id)));
            Ok(match cexpr {
                // Statically known
                ClassExpr::Id(ast_defs::Id(_, cname)) => {
                    let cid = class::ClassType::<'arena>::from_ast_name_and_mangle(alloc, &cname);
                    emit_symbol_refs::add_class(e, cid.clone());
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![instr::nulluninit(alloc), instr::nulluninit(alloc)],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                generics,
                                instr::fcallclsmethodd(alloc, fcall_args, method_id, cid),
                            ],
                        ),
                    )
                }
                ClassExpr::Special(clsref) => {
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![instr::nulluninit(alloc), instr::nulluninit(alloc)],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                generics,
                                instr::fcallclsmethodsd(alloc, fcall_args, clsref, method_id),
                            ],
                        ),
                    )
                }
                ClassExpr::Expr(expr) => {
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![instr::nulluninit(alloc), instr::nulluninit(alloc)],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                generics,
                                instr::string(alloc, method_id.to_raw_string()),
                                emit_expr(e, env, &expr)?,
                                instr::classgetc(alloc),
                                instr::fcallclsmethod(
                                    alloc,
                                    IsLogAsDynamicCallOp::DontLogAsDynamicCall,
                                    fcall_args,
                                ),
                            ],
                        ),
                    )
                }
                ClassExpr::Reified(instrs) => {
                    let tmp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::nulluninit(alloc),
                                instr::nulluninit(alloc),
                                instrs,
                                instr::popl(alloc, tmp),
                            ],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::string(alloc, method_id.to_raw_string()),
                                instr::pushl(alloc, tmp),
                                instr::classgetc(alloc),
                                instr::fcallclsmethod(
                                    alloc,
                                    IsLogAsDynamicCallOp::LogAsDynamicCall,
                                    fcall_args,
                                ),
                            ],
                        ),
                    )
                }
            })
        }
        E_::ClassGet(c) if c.as_ref().2 == ast::PropOrMethod::IsMethod => {
            // Case Foo::bar(...).
            let (cid, cls_get_expr, _) = &**c;
            let mut cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
            if let ClassExpr::Id(ast_defs::Id(_, name)) = &cexpr {
                if let Some(reified_var_cexpr) = get_reified_var_cexpr(env, pos, &name)? {
                    cexpr = reified_var_cexpr;
                }
            }
            let emit_meth_name = |e: &mut Emitter<'arena, 'decl>| match &cls_get_expr {
                ast::ClassGetExpr::CGstring((pos, id)) => Ok(emit_pos_then(
                    alloc,
                    pos,
                    instr::cgetl(alloc, Local::Named(Str::new_str(alloc, id.as_str()))),
                )),
                ast::ClassGetExpr::CGexpr(expr) => emit_expr(e, env, expr),
            };
            Ok(match cexpr {
                ClassExpr::Id(cid) => {
                    let tmp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::nulluninit(alloc),
                                instr::nulluninit(alloc),
                                emit_meth_name(e)?,
                                instr::popl(alloc, tmp),
                            ],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::pushl(alloc, tmp),
                                emit_known_class_id(alloc, e, &cid),
                                instr::fcallclsmethod(
                                    alloc,
                                    IsLogAsDynamicCallOp::LogAsDynamicCall,
                                    fcall_args,
                                ),
                            ],
                        ),
                    )
                }
                ClassExpr::Special(clsref) => {
                    let tmp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::nulluninit(alloc),
                                instr::nulluninit(alloc),
                                emit_meth_name(e)?,
                                instr::popl(alloc, tmp),
                            ],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::pushl(alloc, tmp),
                                instr::fcallclsmethods(alloc, fcall_args, clsref),
                            ],
                        ),
                    )
                }
                ClassExpr::Expr(expr) => {
                    let cls = e.local_gen_mut().get_unnamed();
                    let meth = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::nulluninit(alloc),
                                instr::nulluninit(alloc),
                                emit_expr(e, env, &expr)?,
                                instr::popl(alloc, cls),
                                emit_meth_name(e)?,
                                instr::popl(alloc, meth),
                            ],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::pushl(alloc, meth),
                                instr::pushl(alloc, cls),
                                instr::classgetc(alloc),
                                instr::fcallclsmethod(
                                    alloc,
                                    IsLogAsDynamicCallOp::LogAsDynamicCall,
                                    fcall_args,
                                ),
                            ],
                        ),
                    )
                }
                ClassExpr::Reified(instrs) => {
                    let cls = e.local_gen_mut().get_unnamed();
                    let meth = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::nulluninit(alloc),
                                instr::nulluninit(alloc),
                                instrs,
                                instr::popl(alloc, cls),
                                emit_meth_name(e)?,
                                instr::popl(alloc, meth),
                            ],
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::pushl(alloc, meth),
                                instr::pushl(alloc, cls),
                                instr::classgetc(alloc),
                                instr::fcallclsmethod(
                                    alloc,
                                    IsLogAsDynamicCallOp::LogAsDynamicCall,
                                    fcall_args,
                                ),
                            ],
                        ),
                    )
                }
            })
        }
        E_::Id(id) => {
            let FcallArgs(flags, num_args, _, _, _, _, _) = fcall_args;
            let fq_id = match string_utils::strip_global_ns(&id.1) {
                "min" if num_args == 2 && !flags.contains(FcallFlags::HAS_UNPACK) => {
                    function::FunctionType::<'arena>::from_ast_name(alloc, "__SystemLib\\min2")
                }
                "max" if num_args == 2 && !flags.contains(FcallFlags::HAS_UNPACK) => {
                    function::FunctionType::<'arena>::from_ast_name(alloc, "__SystemLib\\max2")
                }
                _ => function::FunctionType(Str::new_str(
                    alloc,
                    string_utils::strip_global_ns(&id.1),
                )),
            };
            let generics = emit_generics(e, env, &mut fcall_args)?;
            Ok((
                InstrSeq::gather(
                    alloc,
                    vec![instr::nulluninit(alloc), instr::nulluninit(alloc)],
                ),
                InstrSeq::gather(
                    alloc,
                    vec![generics, instr::fcallfuncd(alloc, fcall_args, fq_id)],
                ),
            ))
        }
        E_::String(s) => {
            // TODO(hrust) should be able to accept `let fq_id = function::from_raw_string(s);`
            let fq_id = function::FunctionType(Str::new_str(alloc, s.to_string().as_str()));
            let generics = emit_generics(e, env, &mut fcall_args)?;
            Ok((
                InstrSeq::gather(
                    alloc,
                    vec![instr::nulluninit(alloc), instr::nulluninit(alloc)],
                ),
                InstrSeq::gather(
                    alloc,
                    vec![generics, instr::fcallfuncd(alloc, fcall_args, fq_id)],
                ),
            ))
        }
        _ => emit_fcall_func(e, env, expr, fcall_args, caller_readonly_opt),
    }
}

fn get_reified_var_cexpr<'a, 'arena, 'decl>(
    env: &Env<'a, 'arena>,
    pos: &Pos,
    name: &str,
) -> Result<Option<ClassExpr<'arena>>> {
    let alloc = env.arena;
    Ok(emit_reified_type_opt(env, pos, name)?.map(|instrs| {
        ClassExpr::Reified(InstrSeq::gather(
            alloc,
            vec![
                instrs,
                instr::basec(alloc, 0, MemberOpMode::Warn),
                instr::querym(
                    alloc,
                    1,
                    QueryOp::CGet,
                    MemberKey::ET(Str::from("classname"), ReadonlyOp::Any),
                ),
            ],
        ))
    }))
}

fn emit_args_inout_setters<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    args: &[(ParamKind, ast::Expr)],
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    let alloc = env.arena;
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
        use ast::Expr_ as E_;
        let alloc = env.arena;
        match (pk, &arg.2) {
            // inout $var
            (ParamKind::Pinout(_), E_::Lvar(l)) => {
                let local = get_local(e, env, &l.0, local_id::get_name(&l.1))?;
                let move_instrs = if !env.flags.contains(hhbc_by_ref_env::Flags::IN_TRY)
                    && inout_locals::should_move_local_value(&local, aliases)
                {
                    InstrSeq::gather(alloc, vec![instr::null(alloc), instr::popl(alloc, local)])
                } else {
                    instr::empty(alloc)
                };
                Ok((
                    InstrSeq::gather(alloc, vec![instr::cgetl(alloc, local), move_instrs]),
                    instr::popl(alloc, local),
                ))
            }
            // inout $arr[...][...]
            (ParamKind::Pinout(_), E_::ArrayGet(ag)) => {
                let array_get_result = emit_array_get_(
                    e,
                    env,
                    &arg.1,
                    None,
                    QueryOp::InOut,
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
                            Some(MemberOpMode::Define),
                            QueryOp::InOut,
                            &ag.0,
                            ag.1.as_ref(),
                            true,
                            false,
                        )?
                        .0;
                        let (mk, warninstr) = get_elem_member_key(e, env, 0, ag.1.as_ref(), false)?;
                        let setter = InstrSeq::gather(
                            alloc,
                            vec![
                                warninstr,
                                setter_base,
                                instr::setm(alloc, 0, mk),
                                instr::popc(alloc),
                            ],
                        );
                        (instrs, setter)
                    }
                    ArrayGetInstr::Inout { load, store } => {
                        let (mut ld, mut st) = (vec![], vec![store]);
                        for (instr, local_kind_opt) in load.into_iter() {
                            match local_kind_opt {
                                None => ld.push(instr),
                                Some((l, kind)) => {
                                    let unset = instr::unsetl(alloc, l);
                                    let set = match kind {
                                        StoredValueKind::Expr => instr::setl(alloc, l),
                                        _ => instr::popl(alloc, l),
                                    };
                                    ld.push(instr);
                                    ld.push(set);
                                    st.push(unset);
                                }
                            }
                        }
                        (InstrSeq::gather(alloc, ld), InstrSeq::gather(alloc, st))
                    }
                })
            }
            (ParamKind::Pinout(_), _) => Err(unrecoverable(
                "emit_arg_and_inout_setter: Unexpected inout expression type",
            )),
            _ => Ok((emit_expr(e, env, arg)?, instr::empty(alloc))),
        }
    }
    let (instr_args, instr_setters): (Vec<InstrSeq<'_>>, Vec<InstrSeq<'_>>) = args
        .iter()
        .enumerate()
        .map(|(i, (pk, arg))| emit_arg_and_inout_setter(e, env, i, pk, arg, &aliases))
        .collect::<Result<Vec<_>>>()?
        .into_iter()
        .unzip();
    let instr_args = InstrSeq::gather(alloc, instr_args);
    let instr_setters = InstrSeq::gather(alloc, instr_setters);
    if has_inout_arg(args) {
        let retval = e.local_gen_mut().get_unnamed();
        Ok((
            instr_args,
            InstrSeq::gather(
                alloc,
                vec![
                    instr::popl(alloc, retval),
                    instr_setters,
                    instr::pushl(alloc, retval),
                ],
            ),
        ))
    } else {
        Ok((instr_args, instr::empty(alloc)))
    }
}

/// You can think of Hack as having two "types" of function calls:
/// - Normal function calls, where the parameters might have non-standard calling convention
/// - "Special calls" where we expect all parameters to be passed in normally, mainly (if not
///   exclusively) object constructors
///
/// This function abstracts over these two kinds of calls: given a list of arguments and two
/// predicates (is this an `inout` argument, is this `readonly`) we build up an `FcallArgs` for the
/// given function call.
fn get_fcall_args_common<'arena, 'decl, T>(
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
) -> FcallArgs<'arena> {
    let mut flags = FcallFlags::default();
    flags.set(FcallFlags::HAS_UNPACK, uarg.is_some());
    flags.set(FcallFlags::LOCK_WHILE_UNWINDING, lock_while_unwinding);
    flags.set(FcallFlags::ENFORCE_MUTABLE_RETURN, !readonly_return);
    flags.set(FcallFlags::ENFORCE_READONLY_THIS, readonly_this);
    let readonly_args = if args.iter().any(readonly_predicate) {
        Slice::fill_iter(alloc, args.iter().map(readonly_predicate))
    } else {
        Slice::empty()
    };
    FcallArgs::new(
        flags,
        1 + args.iter().filter(|e| is_inout_arg(e)).count(),
        Slice::fill_iter(alloc, args.iter().map(is_inout_arg)),
        readonly_args,
        async_eager_label,
        args.len(),
        context
            .map(|s| bumpalo::collections::String::from_str_in(s.as_str(), alloc).into_bump_str()),
    )
}

fn get_fcall_args_no_inout<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    args: &[ast::Expr],
    uarg: Option<&ast::Expr>,
    async_eager_label: Option<Label>,
    context: Option<String>,
    lock_while_unwinding: bool,
    readonly_return: bool,
    readonly_this: bool,
) -> FcallArgs<'arena> {
    get_fcall_args_common(
        alloc,
        args,
        uarg,
        async_eager_label,
        context,
        lock_while_unwinding,
        readonly_return,
        readonly_this,
        |expr| is_readonly_expr(expr),
        |_| false,
    )
}

fn get_fcall_args<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    args: &[(ParamKind, ast::Expr)],
    uarg: Option<&ast::Expr>,
    async_eager_label: Option<Label>,
    context: Option<String>,
    lock_while_unwinding: bool,
    readonly_return: bool,
    readonly_this: bool,
) -> FcallArgs<'arena> {
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
    args: &[(ParamKind, ast::Expr)],
    uarg: Option<&ast::Expr>,
    lower_fq_name: &str,
) -> Result<Option<InstrSeq<'arena>>> {
    use ast::{Expr as E, Expr_ as E_};
    let alloc = env.arena;
    let nargs = args.len() + uarg.map_or(0, |_| 1);
    let fun_and_clsmeth_disabled = e
        .options()
        .hhvm
        .hack_lang
        .flags
        .contains(LangFlags::DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS);
    match (lower_fq_name, args) {
        (id, _) if id == special_functions::ECHO => Ok(Some(InstrSeq::gather(
            alloc,
            args.iter()
                .enumerate()
                .map(|(i, arg)| {
                    Ok(InstrSeq::gather(
                        alloc,
                        vec![
                            emit_expr(e, env, expect_normal_paramkind(arg)?)?,
                            emit_pos(alloc, pos),
                            instr::print(alloc),
                            if i == nargs - 1 {
                                instr::empty(alloc)
                            } else {
                                instr::popc(alloc)
                            },
                        ],
                    ))
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
                    "\\hh\\invariant_violation".into(),
                )),
            );
            let call = ast::Expr(
                (),
                pos.clone(),
                ast::Expr_::mk_call(expr_id, vec![], args[1..].to_owned(), uarg.cloned()),
            );
            let ignored_expr = emit_ignored_expr(e, env, &Pos::make_none(), &call)?;
            Ok(Some(InstrSeq::gather(
                alloc,
                vec![
                    emit_expr(e, env, expect_normal_paramkind(&args[0])?)?,
                    instr::jmpnz(alloc, l),
                    ignored_expr,
                    emit_fatal::emit_fatal_runtime(alloc, pos, "invariant_violation"),
                    instr::label(alloc, l),
                    instr::null(alloc),
                ],
            )))
        }
        ("class_exists", &[ref arg1, ..])
        | ("trait_exists", &[ref arg1, ..])
        | ("interface_exists", &[ref arg1, ..])
            if nargs == 1 || nargs == 2 =>
        {
            let class_kind = match lower_fq_name {
                "class_exists" => ClassishKind::Class,
                "interface_exists" => ClassishKind::Interface,
                "trait_exists" => ClassishKind::Trait,
                _ => return Err(unrecoverable("emit_special_function: class_kind")),
            };
            Ok(Some(InstrSeq::gather(
                alloc,
                vec![
                    emit_expr(e, env, expect_normal_paramkind(arg1)?)?,
                    instr::cast_string(alloc),
                    if nargs == 1 {
                        instr::true_(alloc)
                    } else {
                        InstrSeq::gather(
                            alloc,
                            vec![
                                emit_expr(e, env, expect_normal_paramkind(&args[1])?)?,
                                instr::cast_bool(alloc),
                            ],
                        )
                    },
                    instr::oodeclexists(alloc, class_kind),
                ],
            )))
        }
        ("exit", _) | ("die", _) if nargs == 0 || nargs == 1 => Ok(Some(emit_exit(
            e,
            env,
            args.first().map(expect_normal_paramkind).transpose()?,
        )?)),
        ("HH\\fun", _) => {
            if fun_and_clsmeth_disabled {
                match args {
                    [(_, ast::Expr(_, _, ast::Expr_::String(func_name)))] => {
                        Err(emit_fatal::raise_fatal_parse(
                            pos,
                            format!(
                                "`fun()` is disabled; switch to first-class references like `{}<>`",
                                func_name
                            ),
                        ))
                    }
                    _ => Err(emit_fatal::raise_fatal_runtime(
                        pos,
                        "Constant string expected in fun()",
                    )),
                }
            } else if nargs != 1 {
                Err(emit_fatal::raise_fatal_runtime(
                    pos,
                    format!(
                        "fun() expects exactly 1 parameter, {} given",
                        nargs.to_string()
                    ),
                ))
            } else {
                match args {
                    // `inout` is dropped here, but it should be impossible to have an expression
                    // like: `foo(inout "literal")`
                    [(_, ast::Expr(_, _, ast::Expr_::String(func_name)))] => {
                        Ok(Some(emit_hh_fun(
                            e,
                            env,
                            pos,
                            &[],
                            // FIXME: This is not safe--string literals are binary strings.
                            // There's no guarantee that they're valid UTF-8.
                            unsafe { std::str::from_utf8_unchecked(func_name.as_slice()) },
                        )?))
                    }
                    _ => Err(emit_fatal::raise_fatal_runtime(
                        pos,
                        "Constant string expected in fun()",
                    )),
                }
            }
        }
        ("__systemlib\\meth_caller", _) => {
            // used by meth_caller() to directly emit func ptr
            if nargs != 1 {
                return Err(emit_fatal::raise_fatal_runtime(
                    pos,
                    format!(
                        "fun() expects exactly 1 parameter, {} given",
                        nargs.to_string()
                    ),
                ));
            }
            match args {
                // `inout` is dropped here, but it should be impossible to have an expression
                // like: `foo(inout "literal")`
                [(_, E(_, _, E_::String(ref func_name)))] => Ok(Some(instr::resolve_meth_caller(
                    alloc,
                    function::FunctionType(Str::new_str(
                        alloc,
                        string_utils::strip_global_ns(
                            // FIXME: This is not safe--string literals are binary strings.
                            // There's no guarantee that they're valid UTF-8.
                            unsafe { std::str::from_utf8_unchecked(func_name.as_slice()) },
                        ),
                    )),
                ))),
                _ => Err(emit_fatal::raise_fatal_runtime(
                    pos,
                    "Constant string expected in fun()",
                )),
            }
        }
        ("__systemlib\\__debugger_is_uninit", _) => {
            if nargs != 1 {
                Err(emit_fatal::raise_fatal_runtime(
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
                    [(_, E(_, _, E_::Lvar(id)))] => Ok(Some(instr::isunsetl(
                        alloc,
                        get_local(e, env, pos, id.name())?,
                    ))),
                    _ => Err(emit_fatal::raise_fatal_runtime(
                        pos,
                        "Local variable expected in __debugger_is_uninit()",
                    )),
                }
            }
        }
        ("__SystemLib\\get_enum_member_by_label", _) if e.systemlib() => {
            let local = match args {
                [(pk, E(_, _, E_::Lvar(id)))] => {
                    ensure_normal_paramkind(pk)?;
                    get_local(e, env, pos, id.name())
                }
                _ => Err(emit_fatal::raise_fatal_runtime(
                    pos,
                    "Argument must be the label argument",
                )),
            }?;
            Ok(Some(InstrSeq::gather(
                alloc,
                vec![instr::lateboundcls(alloc), instr::clscnsl(alloc, local)],
            )))
        }
        ("HH\\inst_meth", _) => match args {
            [obj_expr, method_name] => Ok(Some(emit_inst_meth(
                e,
                env,
                expect_normal_paramkind(obj_expr)?,
                expect_normal_paramkind(method_name)?,
            )?)),
            _ => Err(emit_fatal::raise_fatal_runtime(
                pos,
                format!(
                    "inst_meth() expects exactly 2 parameters, {} given",
                    nargs.to_string()
                ),
            )),
        },
        ("HH\\class_meth", _) if fun_and_clsmeth_disabled => Err(emit_fatal::raise_fatal_parse(
            pos,
            "`class_meth()` is disabled; switch to first-class references like `C::bar<>`",
        )),
        ("HH\\class_meth", &[(ref pk_cls, ref cls), (ref pk_meth, ref meth), ..]) if nargs == 2 => {
            if meth.2.is_string() {
                if cls.2.is_string()
                    || cls
                        .2
                        .as_class_const()
                        .map_or(false, |(_, (_, id))| string_utils::is_class(id))
                    || cls
                        .2
                        .as_id()
                        .map_or(false, |ast_defs::Id(_, id)| id == pseudo_consts::G__CLASS__)
                {
                    ensure_normal_paramkind(pk_cls)?;
                    ensure_normal_paramkind(pk_meth)?;
                    return Ok(Some(emit_class_meth(e, env, cls, meth)?));
                }
            }
            Err(emit_fatal::raise_fatal_runtime(
                pos,
                concat!(
                    "class_meth() expects a literal class name or ::class constant, ",
                    "followed by a constant string that refers to a static method ",
                    "on that class"
                ),
            ))
        }
        ("HH\\class_meth", _) => Err(emit_fatal::raise_fatal_runtime(
            pos,
            format!(
                "class_meth() expects exactly 2 parameters, {} given",
                nargs.to_string()
            ),
        )),
        ("HH\\global_set", _) => match *args {
            [ref gkey, ref gvalue] => Ok(Some(InstrSeq::gather(
                alloc,
                vec![
                    emit_expr(e, env, expect_normal_paramkind(gkey)?)?,
                    emit_expr(e, env, expect_normal_paramkind(gvalue)?)?,
                    emit_pos(alloc, pos),
                    instr::setg(alloc),
                    instr::popc(alloc),
                    instr::null(alloc),
                ],
            ))),
            _ => Err(emit_fatal::raise_fatal_runtime(
                pos,
                format!(
                    "global_set() expects exactly 2 parameters, {} given",
                    nargs.to_string()
                ),
            )),
        },
        ("HH\\global_unset", _) => match *args {
            [ref gkey] => Ok(Some(InstrSeq::gather(
                alloc,
                vec![
                    emit_expr(e, env, expect_normal_paramkind(gkey)?)?,
                    emit_pos(alloc, pos),
                    instr::unsetg(alloc),
                    instr::null(alloc),
                ],
            ))),
            _ => Err(emit_fatal::raise_fatal_runtime(
                pos,
                format!(
                    "global_unset() expects exactly 1 parameter, {} given",
                    nargs.to_string()
                ),
            )),
        },
        ("__hhvm_internal_whresult", &[(ref pk, E(_, _, E_::Lvar(ref param)))])
            if e.systemlib() =>
        {
            ensure_normal_paramkind(pk)?;
            Ok(Some(InstrSeq::gather(
                alloc,
                vec![
                    instr::cgetl(
                        alloc,
                        Local::Named(Str::new_str(alloc, local_id::get_name(&param.1))),
                    ),
                    instr::whresult(alloc),
                ],
            )))
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
        _ => Ok(
            match (args, istype_op(lower_fq_name), is_isexp_op(lower_fq_name)) {
                (&[ref arg_expr], _, Some(ref h)) => {
                    let is_expr = emit_is(e, env, pos, &h)?;
                    Some(InstrSeq::gather(
                        alloc,
                        vec![
                            emit_expr(e, env, expect_normal_paramkind(arg_expr)?)?,
                            is_expr,
                        ],
                    ))
                }
                (&[(ref pk, E(_, _, E_::Lvar(ref arg_id)))], Some(i), _)
                    if superglobals::is_any_global(arg_id.name()) =>
                {
                    ensure_normal_paramkind(pk)?;
                    Some(InstrSeq::gather(
                        alloc,
                        vec![
                            emit_local(e, env, BareThisOp::NoNotice, &arg_id)?,
                            emit_pos(alloc, pos),
                            instr::istypec(alloc, i),
                        ],
                    ))
                }
                (&[(ref pk, E(_, _, E_::Lvar(ref arg_id)))], Some(i), _)
                    if !is_local_this(env, &arg_id.1) =>
                {
                    ensure_normal_paramkind(pk)?;
                    Some(instr::istypel(
                        alloc,
                        get_local(e, env, &arg_id.0, &(arg_id.1).1)?,
                        i,
                    ))
                }
                (&[(ref pk, ref arg_expr)], Some(i), _) => {
                    ensure_normal_paramkind(pk)?;
                    Some(InstrSeq::gather(
                        alloc,
                        vec![
                            emit_expr(e, env, &arg_expr)?,
                            emit_pos(alloc, pos),
                            instr::istypec(alloc, i),
                        ],
                    ))
                }
                _ => match get_call_builtin_func_info(e, lower_fq_name) {
                    Some((nargs, i)) if nargs == args.len() => Some(InstrSeq::gather(
                        alloc,
                        vec![
                            emit_exprs_and_error_on_inout(e, env, args, lower_fq_name)?,
                            emit_pos(alloc, pos),
                            instr::instr(alloc, i),
                        ],
                    )),
                    _ => None,
                },
            },
        ),
    }
}

fn emit_inst_meth<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    obj_expr: &ast::Expr,
    method_name: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let instrs = InstrSeq::gather(
        alloc,
        vec![
            emit_expr(e, env, obj_expr)?,
            emit_expr(e, env, method_name)?,
            instr::new_vec_array(alloc, 2),
        ],
    );
    Ok(instrs)
}

fn emit_class_meth<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    cls: &ast::Expr,
    meth: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    use ast::Expr_ as E_;
    let alloc = env.arena;
    if e.options()
        .hhvm
        .flags
        .contains(HhvmFlags::EMIT_CLS_METH_POINTERS)
    {
        let method_id = match &meth.2 {
            E_::String(method_name) => {
                method::MethodType(Str::new_str(alloc, method_name.to_string().as_str()))
            }
            _ => return Err(unrecoverable("emit_class_meth: unhandled method")),
        };
        if let Some((cid, (_, id))) = cls.2.as_class_const() {
            if string_utils::is_class(id) {
                return emit_class_meth_native(e, env, &cls.1, cid, method_id, &[]);
            }
        }
        if let Some(ast_defs::Id(_, s)) = cls.2.as_id() {
            if s == pseudo_consts::G__CLASS__ {
                return Ok(instr::resolveclsmethods(
                    alloc,
                    SpecialClsRef::Self_,
                    method_id,
                ));
            }
        }
        if let Some(class_name) = cls.2.as_string() {
            return Ok(instr::resolveclsmethodd(
                alloc,
                class::ClassType(Str::new_str(
                    alloc,
                    string_utils::strip_global_ns(
                        // FIXME: This is not safe--string literals are binary strings.
                        // There's no guarantee that they're valid UTF-8.
                        unsafe { std::str::from_utf8_unchecked(class_name.as_slice()) },
                    ),
                )),
                method_id,
            ));
        }
        Err(unrecoverable("emit_class_meth: unhandled method"))
    } else {
        let instrs = InstrSeq::gather(
            alloc,
            vec![
                emit_expr(e, env, cls)?,
                emit_expr(e, env, meth)?,
                instr::new_vec_array(alloc, 2),
            ],
        );
        Ok(instrs)
    }
}

fn emit_class_meth_native<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    cid: &ast::ClassId,
    method_id: MethodId<'arena>,
    targs: &[ast::Targ],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let mut cexpr = ClassExpr::class_id_to_class_expr(e, false, true, &env.scope, cid);
    if let ClassExpr::Id(ast_defs::Id(_, name)) = &cexpr {
        if let Some(reified_var_cexpr) = get_reified_var_cexpr(env, pos, &name)? {
            cexpr = reified_var_cexpr;
        }
    }
    let has_generics = has_non_tparam_generics_targs(env, targs);
    let mut emit_generics = || -> Result<InstrSeq<'arena>> {
        emit_reified_targs(
            e,
            env,
            pos,
            &targs.iter().map(|targ| &targ.1).collect::<Vec<_>>(),
        )
    };
    Ok(match cexpr {
        ClassExpr::Id(ast_defs::Id(_, name)) if !has_generics => instr::resolveclsmethodd(
            alloc,
            class::ClassType::<'arena>::from_ast_name_and_mangle(alloc, &name),
            method_id,
        ),
        ClassExpr::Id(ast_defs::Id(_, name)) => InstrSeq::gather(
            alloc,
            vec![
                emit_generics()?,
                instr::resolverclsmethodd(
                    alloc,
                    class::ClassType::<'arena>::from_ast_name_and_mangle(alloc, &name),
                    method_id,
                ),
            ],
        ),
        ClassExpr::Special(clsref) if !has_generics => {
            instr::resolveclsmethods(alloc, clsref, method_id)
        }
        ClassExpr::Special(clsref) => InstrSeq::gather(
            alloc,
            vec![
                emit_generics()?,
                instr::resolverclsmethods(alloc, clsref, method_id),
            ],
        ),
        ClassExpr::Reified(instrs) if !has_generics => InstrSeq::gather(
            alloc,
            vec![
                instrs,
                instr::classgetc(alloc),
                instr::resolveclsmethod(alloc, method_id),
            ],
        ),
        ClassExpr::Reified(instrs) => InstrSeq::gather(
            alloc,
            vec![
                instrs,
                instr::classgetc(alloc),
                emit_generics()?,
                instr::resolverclsmethod(alloc, method_id),
            ],
        ),
        ClassExpr::Expr(_) => {
            return Err(unrecoverable(
                "emit_class_meth_native: ClassExpr::Expr should be impossible",
            ));
        }
    })
}

fn get_call_builtin_func_info<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    id: impl AsRef<str>,
) -> Option<(usize, Instruct<'arena>)> {
    use {Instruct::*, InstructGet::*, InstructIsset::*, InstructMisc::*, InstructOperator::*};
    match id.as_ref() {
        "array_key_exists" => Some((2, IMisc(AKExists))),
        "hphp_array_idx" => Some((3, IMisc(ArrayIdx))),
        "intval" => Some((1, IOp(CastInt))),
        "boolval" => Some((1, IOp(CastBool))),
        "strval" => Some((1, IOp(CastString))),
        "floatval" | "doubleval" => Some((1, IOp(CastDouble))),
        "HH\\vec" => Some((1, IOp(CastVec))),
        "HH\\keyset" => Some((1, IOp(CastKeyset))),
        "HH\\dict" => Some((1, IOp(CastDict))),
        "HH\\varray" => Some((1, IOp(CastVec))),
        "HH\\darray" => Some((1, IOp(CastDict))),
        "HH\\ImplicitContext\\_Private\\set_implicit_context_by_value" if e.systemlib() => {
            Some((1, IMisc(SetImplicitContextByValue)))
        }
        // TODO: enforce that this returns readonly
        "HH\\global_readonly_get" => Some((1, IGet(CGetG))),
        "HH\\global_get" => Some((1, IGet(CGetG))),
        "HH\\global_isset" => Some((1, IIsset(IssetG))),
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
        ast::FunctionPtrId::FPClassConst(cid, method_id) => {
            // TODO(hrust) should accept `let method_id = method::MethodType::from_ast_name(&(cc.1).1);`
            let method_id = method::MethodType(Str::new_str(
                alloc,
                string_utils::strip_global_ns(&method_id.1),
            ));
            emit_class_meth_native(e, env, pos, cid, method_id, targs)?
        }
    };
    Ok(emit_pos_then(alloc, pos, instrs))
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
        let generics = emit_reified_targs(
            e,
            env,
            pos,
            targs
                .iter()
                .map(|targ| &targ.1)
                .collect::<Vec<_>>()
                .as_slice(),
        )?;
        Ok(InstrSeq::gather(
            alloc,
            vec![
                generics,
                instr::resolve_rfunc(alloc, function::FunctionType(Str::new_str(alloc, fname))),
            ],
        ))
    } else {
        Ok(instr::resolve_func(
            alloc,
            function::FunctionType(Str::new_str(alloc, fname)),
        ))
    }
}

fn emit_is<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    h: &ast::Hint,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let (ts_instrs, is_static) = emit_reified_arg(e, env, pos, true, h)?;
    Ok(if is_static {
        match &*h.1 {
            aast_defs::Hint_::Happly(ast_defs::Id(_, id), hs)
                if hs.is_empty() && string_utils::strip_hh_ns(&id) == typehints::THIS =>
            {
                instr::islateboundcls(alloc)
            }
            _ => InstrSeq::gather(
                alloc,
                vec![
                    get_type_structure_for_hint(e, &[], &IndexSet::new(), h)?,
                    instr::is_type_structc_resolve(alloc),
                ],
            ),
        }
    } else {
        InstrSeq::gather(
            alloc,
            vec![ts_instrs, instr::is_type_structc_dontresolve(alloc)],
        )
    })
}

fn istype_op(id: impl AsRef<str>) -> Option<IstypeOp> {
    use IstypeOp::*;
    match id.as_ref() {
        "is_int" | "is_integer" | "is_long" => Some(OpInt),
        "is_bool" => Some(OpBool),
        "is_float" | "is_real" | "is_double" => Some(OpDbl),
        "is_string" => Some(OpStr),
        "is_object" => Some(OpObj),
        "is_null" => Some(OpNull),
        "is_scalar" => Some(OpScalar),
        "HH\\is_keyset" => Some(OpKeyset),
        "HH\\is_dict" => Some(OpDict),
        "HH\\is_vec" => Some(OpVec),
        "HH\\is_varray" => Some(OpVec),
        "HH\\is_darray" => Some(OpDict),
        "HH\\is_any_array" => Some(OpArrLike),
        "HH\\is_class_meth" => Some(OpClsMeth),
        "HH\\is_fun" => Some(OpFunc),
        "HH\\is_php_array" => Some(OpLegacyArrLike),
        "HH\\is_array_marked_legacy" => Some(OpLegacyArrLike),
        "HH\\is_class" => Some(OpClass),
        _ => None,
    }
}

fn is_isexp_op(lower_fq_id: impl AsRef<str>) -> Option<ast::Hint> {
    let h = |s: &str| {
        Some(ast::Hint::new(
            Pos::make_none(),
            ast::Hint_::mk_happly(ast::Id(Pos::make_none(), s.into()), vec![]),
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
    let alloc = env.arena;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            emit_expr(e, env, expr)?,
            emit_pos(alloc, pos),
            instr::eval(alloc),
        ],
    ))
}

#[allow(clippy::needless_lifetimes)]
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

fn emit_call_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    async_eager_label: Option<Label>,
    readonly_return: bool,
    (expr, targs, args, uarg): &(
        ast::Expr,
        Vec<ast::Targ>,
        Vec<(ParamKind, ast::Expr)>,
        Option<ast::Expr>,
    ),
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let jit_enable_rename_function = e
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION);
    use {ast::Expr as E, ast::Expr_ as E_};
    match (&expr.2, &args[..], uarg) {
        (E_::Id(id), [(pk, E(_, _, E_::String(data)))], None)
            if id.1 == special_functions::HHAS_ADATA =>
        {
            ensure_normal_paramkind(pk)?;
            // FIXME: This is not safe--string literals are binary strings.
            // There's no guarantee that they're valid UTF-8.
            let v = TypedValue::mk_hhas_adata(
                alloc.alloc_str(unsafe { std::str::from_utf8_unchecked(data.as_ref()) }),
            );
            Ok(emit_pos_then(alloc, pos, instr::typedvalue(alloc, v)))
        }
        (E_::Id(id), _, None) if id.1 == pseudo_functions::ISSET => {
            emit_call_isset_exprs(e, env, pos, args)
        }
        (E_::Id(id), args, None)
            if (id.1 == fb::IDX || id.1 == fb::IDXREADONLY)
                && !jit_enable_rename_function
                && (args.len() == 2 || args.len() == 3) =>
        {
            emit_idx(e, env, pos, args)
        }
        (E_::Id(id), [(pk, arg1)], None) if id.1 == emitter_special_functions::EVAL => {
            ensure_normal_paramkind(pk)?;
            emit_eval(e, env, pos, arg1)
        }
        (E_::Id(id), [(pk, arg1)], None)
            if id.1 == emitter_special_functions::SET_FRAME_METADATA =>
        {
            ensure_normal_paramkind(pk)?;
            Ok(InstrSeq::gather(
                alloc,
                vec![
                    emit_expr(e, env, arg1)?,
                    emit_pos(alloc, pos),
                    instr::popl(alloc, Local::Named(Slice::new("$86metadata".as_bytes()))),
                    instr::null(alloc),
                ],
            ))
        }
        (E_::Id(id), [], None)
            if id.1 == pseudo_functions::EXIT || id.1 == pseudo_functions::DIE =>
        {
            let exit = emit_exit(e, env, None)?;
            Ok(emit_pos_then(alloc, pos, exit))
        }
        (E_::Id(id), [(pk, arg1)], None)
            if id.1 == pseudo_functions::EXIT || id.1 == pseudo_functions::DIE =>
        {
            ensure_normal_paramkind(pk)?;
            let exit = emit_exit(e, env, Some(arg1))?;
            Ok(emit_pos_then(alloc, pos, exit))
        }
        (E_::Id(id), [], _)
            if id.1 == emitter_special_functions::SYSTEMLIB_REIFIED_GENERICS
                && e.systemlib()
                && has_reified_types(env) =>
        {
            // Rewrite __systemlib_reified_generics() to $0ReifiedGenerics,
            // but only in systemlib functions that take a reified generic.
            let lvar = E::new(
                (),
                pos.clone(),
                E_::Lvar(Box::new(ast::Lid(
                    pos.clone(),
                    local_id::make_unscoped(string_utils::reified::GENERICS_LOCAL_NAME),
                ))),
            );
            emit_expr(e, env, &lvar)
        }
        (_, _, _) => {
            let instrs = emit_call(
                e,
                env,
                pos,
                expr,
                targs,
                args,
                uarg.as_ref(),
                async_eager_label,
                readonly_return,
            )?;
            Ok(emit_pos_then(alloc, pos, instrs))
        }
    }
}

pub fn emit_reified_generic_instrs<'arena>(
    alloc: &'arena bumpalo::Bump,
    pos: &Pos,
    is_fun: bool,
    index: usize,
) -> Result<InstrSeq<'arena>> {
    let base = if is_fun {
        instr::basel(
            alloc,
            Local::Named(Str::new_str(
                alloc,
                string_utils::reified::GENERICS_LOCAL_NAME,
            )),
            MemberOpMode::Warn,
            ReadonlyOp::Any,
        )
    } else {
        InstrSeq::gather(
            alloc,
            vec![
                instr::checkthis(alloc),
                instr::baseh(alloc),
                instr::dim_warn_pt(
                    alloc,
                    prop::from_raw_string(alloc, string_utils::reified::PROP_NAME),
                    ReadonlyOp::Any,
                ),
            ],
        )
    };
    Ok(emit_pos_then(
        alloc,
        pos,
        InstrSeq::gather(
            alloc,
            vec![
                base,
                instr::querym(
                    alloc,
                    0,
                    QueryOp::CGet,
                    MemberKey::EI(index.try_into().unwrap(), ReadonlyOp::Any),
                ),
            ],
        ),
    ))
}

fn emit_reified_type<'a, 'arena, 'decl>(
    env: &Env<'a, 'arena>,
    pos: &Pos,
    name: &str,
) -> Result<InstrSeq<'arena>> {
    emit_reified_type_opt(env, pos, name)?
        .ok_or_else(|| emit_fatal::raise_fatal_runtime(&Pos::make_none(), "Invalid reified param"))
}

fn emit_reified_type_opt<'a, 'arena, 'decl>(
    env: &Env<'a, 'arena>,
    pos: &Pos,
    name: &str,
) -> Result<Option<InstrSeq<'arena>>> {
    let alloc = env.arena;
    let is_in_lambda = env.scope.is_in_lambda();
    let cget_instr = |is_fun, i| {
        instr::cgetl(
            env.arena,
            Local::Named(Str::new_str(
                alloc,
                string_utils::reified::reified_generic_captured_name(is_fun, i).as_str(),
            )),
        )
    };
    let check = |is_soft| -> Result<()> {
        if is_soft {
            Err(emit_fatal::raise_fatal_parse(
                pos,
                format!(
                    "{} is annotated to be a soft reified generic, it cannot be used until the __Soft annotation is removed",
                    name
                ),
            ))
        } else {
            Ok(())
        }
    };
    let emit = |(i, is_soft), is_fun| {
        check(is_soft)?;
        Ok(Some(if is_in_lambda {
            cget_instr(is_fun, i)
        } else {
            emit_reified_generic_instrs(alloc, pos, is_fun, i)?
        }))
    };
    match is_reified_tparam(env, true, name) {
        Some((i, is_soft)) => emit((i, is_soft), true),
        None => match is_reified_tparam(env, false, name) {
            Some((i, is_soft)) => emit((i, is_soft), false),
            None => Ok(None),
        },
    }
}

fn emit_known_class_id<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl>,
    id: &ast_defs::Id,
) -> InstrSeq<'arena> {
    let cid = class::ClassType::from_ast_name(alloc, &id.1);
    let cid_string = instr::string(alloc, cid.to_raw_string());
    emit_symbol_refs::add_class(e, cid);
    InstrSeq::gather(alloc, vec![cid_string, instr::classgetc(alloc)])
}

fn emit_load_class_ref<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    cexpr: ClassExpr<'arena>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let instrs = match cexpr {
        ClassExpr::Special(SpecialClsRef::Self_) => instr::self_(alloc),
        ClassExpr::Special(SpecialClsRef::Static) => instr::lateboundcls(alloc),
        ClassExpr::Special(SpecialClsRef::Parent) => instr::parent(alloc),
        ClassExpr::Id(id) => emit_known_class_id(alloc, e, &id),
        ClassExpr::Expr(expr) => InstrSeq::gather(
            alloc,
            vec![
                emit_pos(alloc, pos),
                emit_expr(e, env, &expr)?,
                instr::classgetc(alloc),
            ],
        ),
        ClassExpr::Reified(instrs) => InstrSeq::gather(
            alloc,
            vec![emit_pos(alloc, pos), instrs, instr::classgetc(alloc)],
        ),
    };
    Ok(emit_pos_then(alloc, pos, instrs))
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
    use HasGenericsOp as H;
    let cexpr = ClassExpr::class_id_to_class_expr(e, false, resolve_self, &env.scope, cid);
    let (cexpr, has_generics) = match &cexpr {
        ClassExpr::Id(ast_defs::Id(_, name)) => match emit_reified_type_opt(env, pos, name)? {
            Some(instrs) => {
                if targs.is_empty() {
                    (ClassExpr::Reified(instrs), H::MaybeGenerics)
                } else {
                    return Err(emit_fatal::raise_fatal_parse(
                        pos,
                        "Cannot have higher kinded reified generics",
                    ));
                }
            }
            None if !has_non_tparam_generics_targs(env, targs) => (cexpr, H::NoGenerics),
            None => (cexpr, H::HasGenerics),
        },
        _ => (cexpr, H::NoGenerics),
    };
    if is_reflection_class_builtin {
        scope::with_unnamed_locals(e, |e| {
            let alloc = e.alloc;
            let instr_args = emit_exprs(e, env, args)?;
            let instr_uargs = match uarg {
                None => instr::empty(alloc),
                Some(uarg) => emit_expr(e, env, uarg)?,
            };
            Ok((
                instr::empty(alloc),
                InstrSeq::gather(alloc, vec![instr_args, instr_uargs]),
                instr::empty(alloc),
            ))
        })
    } else {
        let newobj_instrs = match cexpr {
            ClassExpr::Id(ast_defs::Id(_, cname)) => {
                let id = class::ClassType::<'arena>::from_ast_name_and_mangle(alloc, &cname);
                emit_symbol_refs::add_class(e, id);
                match has_generics {
                    H::NoGenerics => InstrSeq::gather(
                        alloc,
                        vec![emit_pos(alloc, pos), instr::newobjd(alloc, id)],
                    ),
                    H::HasGenerics => InstrSeq::gather(
                        alloc,
                        vec![
                            emit_pos(alloc, pos),
                            emit_reified_targs(
                                e,
                                env,
                                pos,
                                &targs.iter().map(|t| &t.1).collect::<Vec<_>>(),
                            )?,
                            instr::newobjrd(alloc, id),
                        ],
                    ),
                    H::MaybeGenerics => {
                        return Err(unrecoverable(
                            "Internal error: This case should have been transformed",
                        ));
                    }
                }
            }
            ClassExpr::Special(cls_ref) => InstrSeq::gather(
                alloc,
                vec![emit_pos(alloc, pos), instr::newobjs(alloc, cls_ref)],
            ),
            ClassExpr::Reified(instrs) if has_generics == H::MaybeGenerics => InstrSeq::gather(
                alloc,
                vec![instrs, instr::classgetts(alloc), instr::newobjr(alloc)],
            ),
            _ => InstrSeq::gather(
                alloc,
                vec![
                    emit_load_class_ref(e, env, pos, cexpr)?,
                    instr::newobj(alloc),
                ],
            ),
        };
        scope::with_unnamed_locals(e, |e| {
            let alloc = e.alloc;
            let instr_args = emit_exprs(e, env, args)?;
            let instr_uargs = match uarg {
                None => instr::empty(alloc),
                Some(uarg) => emit_expr(e, env, uarg)?,
            };
            Ok((
                instr::empty(alloc),
                InstrSeq::gather(
                    alloc,
                    vec![
                        newobj_instrs,
                        instr::dup(alloc),
                        instr::nulluninit(alloc),
                        instr_args,
                        instr_uargs,
                        emit_pos(alloc, pos),
                        instr::fcallctor(
                            alloc,
                            get_fcall_args_no_inout(
                                alloc,
                                args,
                                uarg.as_ref(),
                                None,
                                env.call_context.clone(),
                                true,
                                true, // we do not need to enforce readonly return for constructors
                                false, // we do not need to enforce readonly this for constructors
                            ),
                        ),
                        instr::popc(alloc),
                        instr::lockobj(alloc),
                    ],
                ),
                instr::empty(alloc),
            ))
        })
    }
}

fn emit_obj_get<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    query_op: QueryOp,
    expr: &ast::Expr,
    prop: &ast::Expr,
    nullflavor: &ast_defs::OgNullFlavor,
    null_coalesce_assignment: bool,
    readonly_get: bool, // obj_get enclosed in readonly expression
) -> Result<(InstrSeq<'arena>, Option<NumParams>)> {
    let readonly_op = if readonly_get {
        ReadonlyOp::Any
    } else {
        ReadonlyOp::Mutable
    };
    let alloc = env.arena;
    if let Some(ast::Lid(pos, id)) = expr.2.as_lvar() {
        if local_id::get_name(&id) == special_idents::THIS
            && nullflavor.eq(&ast_defs::OgNullFlavor::OGNullsafe)
        {
            return Err(emit_fatal::raise_fatal_parse(
                pos,
                "?-> is not allowed with $this",
            ));
        }
    }
    if let Some(ast_defs::Id(_, s)) = prop.2.as_id() {
        if string_utils::is_xhp(s) {
            return Ok((emit_xhp_obj_get(e, env, pos, &expr, s, nullflavor)?, None));
        }
    }
    let mode = if null_coalesce_assignment {
        MemberOpMode::Warn
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
    let final_instr = instr::querym(alloc, num_params, query_op, mk);
    // Don't pop elems/props from the stack during the lookup for null
    // coalesce assignment in case we do a write later.
    let querym_n_unpopped = if null_coalesce_assignment {
        Some(total_stack_size)
    } else {
        None
    };
    let instr = InstrSeq::gather(
        alloc,
        vec![
            base_expr_instrs_begin,
            prop_instrs,
            base_expr_instrs_end,
            emit_pos(alloc, pos),
            base_setup_instrs,
            final_instr,
        ],
    );
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
                let pid: prop::PropType<'arena> = prop::PropType::<'arena>::from_ast_name(
                    alloc,
                    string_utils::strip_global_ns(&name),
                );
                match nullflavor {
                    ast_defs::OgNullFlavor::OGNullthrows => MemberKey::PT(pid, readonly_op),
                    ast_defs::OgNullFlavor::OGNullsafe => MemberKey::QT(pid, readonly_op),
                }
            }
        }
        // Special case for known property name
        ast::Expr_::String(name) => {
            let pid: prop::PropType<'arena> = prop::PropType::<'arena>::from_ast_name(
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
            return Err(emit_fatal::raise_fatal_parse(
                &prop.1,
                "?-> can only be used with scalar property names",
            ));
        }
        MemberKey::PC(_, _) => (mk, emit_expr(e, env, prop)?, 1),
        MemberKey::PL(local, readonly_op) if null_coalesce_assignment => (
            MemberKey::PC(stack_index, readonly_op),
            instr::cgetl(alloc, local),
            1,
        ),
        _ => (mk, instr::empty(alloc), 0),
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
    use ast::Expr as E;
    use ast::Expr_ as E_;
    let f = E(
        (),
        pos.clone(),
        E_::mk_obj_get(
            expr.clone(),
            E(
                (),
                pos.clone(),
                E_::mk_id(ast_defs::Id(pos.clone(), "getAttribute".into())),
            ),
            nullflavor.clone(),
            ast::PropOrMethod::IsMethod,
        ),
    );
    let args = vec![(
        ParamKind::Pnormal,
        E(
            (),
            pos.clone(),
            E_::mk_string(string_utils::clean(s).into()),
        ),
    )];
    emit_call(e, env, pos, &f, &[], &args[..], None, None, false)
}

fn emit_array_get<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    mode: Option<MemberOpMode>,
    query_op: QueryOp,
    base: &ast::Expr,
    elem: Option<&ast::Expr>,
    no_final: bool,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq<'arena>, Option<usize>)> {
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
        (ArrayGetInstr::Inout { .. }, _) => Err(unrecoverable("unexpected inout")),
    }
}

fn emit_array_get_<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    mode: Option<MemberOpMode>,
    query_op: QueryOp,
    base_expr: &ast::Expr,
    elem: Option<&ast::Expr>,
    no_final: bool,
    null_coalesce_assignment: bool,
    inout_param_info: Option<(usize, &inout_locals::AliasInfoMap<'_>)>,
) -> Result<(ArrayGetInstr<'arena>, Option<usize>)> {
    use ast::Expr as E;
    let alloc = env.arena;
    match (base_expr, elem) {
        (E(_, pos, _), None)
            if !env
                .flags
                .contains(hhbc_by_ref_env::Flags::ALLOWS_ARRAY_APPEND) =>
        {
            Err(emit_fatal::raise_fatal_runtime(
                pos,
                "Can't use [] for reading",
            ))
        }
        _ => {
            let local_temp_kind = get_local_temp_kind(env, false, inout_param_info, elem);
            let mode = if null_coalesce_assignment {
                MemberOpMode::Warn
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
                    QueryOp::Isset => BareThisOp::NoNotice,
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
                        instr::empty(alloc)
                    } else if null_coalesce_assignment {
                        querym_n_unpopped = Some(total_stack_size as usize);
                        instr::querym(alloc, 0, query_op, memberkey)
                    } else {
                        instr::querym(alloc, total_stack_size as usize, query_op, memberkey)
                    }
                };
            let instr = match (base_result, local_temp_kind) {
                (ArrayGetBase::Regular(base), None) =>
                // neither base nor expression needs to store anything
                {
                    ArrayGetInstr::Regular(InstrSeq::gather(
                        alloc,
                        vec![
                            warninstr,
                            base.base_instrs,
                            elem_instrs,
                            base.cls_instrs,
                            emit_pos(alloc, outer_pos),
                            base.setup_instrs,
                            make_final(
                                base.base_stack_size + base.cls_stack_size + elem_stack_size,
                                memberkey,
                            ),
                        ],
                    ))
                }
                (ArrayGetBase::Regular(base), Some(local_kind)) => {
                    // base does not need temp locals but index expression does
                    let local = e.local_gen_mut().get_unnamed();
                    let load = vec![
                        // load base and indexer, value of indexer will be saved in local
                        (
                            InstrSeq::gather(
                                alloc,
                                vec![InstrSeq::clone(alloc, &base.base_instrs), elem_instrs],
                            ),
                            Some((local, local_kind)),
                        ),
                        // finish loading the value
                        (
                            InstrSeq::gather(
                                alloc,
                                vec![
                                    warninstr,
                                    base.base_instrs,
                                    emit_pos(alloc, outer_pos),
                                    base.setup_instrs,
                                    make_final(
                                        base.base_stack_size
                                            + base.cls_stack_size
                                            + elem_stack_size,
                                        memberkey,
                                    ),
                                ],
                            ),
                            None,
                        ),
                    ];
                    let store = InstrSeq::gather(
                        alloc,
                        vec![
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
                            instr::popc(alloc),
                        ],
                    );
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
                        InstrSeq::gather(
                            alloc,
                            vec![
                                warninstr,
                                elem_instrs,
                                cls_instrs,
                                emit_pos(alloc, outer_pos),
                                setup_instrs,
                                make_final(
                                    base_stack_size + cls_stack_size + elem_stack_size,
                                    memberkey.clone(),
                                ),
                            ],
                        ),
                        None,
                    ));
                    let store = InstrSeq::gather(
                        alloc,
                        vec![store, instr::setm(alloc, 0, memberkey), instr::popc(alloc)],
                    );
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
                        InstrSeq::gather(
                            alloc,
                            vec![
                                warninstr,
                                cls_instrs,
                                emit_pos(alloc, outer_pos),
                                setup_instrs,
                                make_final(
                                    base_stack_size + cls_stack_size + elem_stack_size,
                                    memberkey,
                                ),
                            ],
                        ),
                        None,
                    ));
                    let store = InstrSeq::gather(
                        alloc,
                        vec![
                            store,
                            instr::setm(alloc, 0, MemberKey::EL(local, ReadonlyOp::Any)),
                            instr::popc(alloc),
                        ],
                    );
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

fn is_special_class_constant_accessed_with_class_id(cname: &ast::ClassId_, id: &str) -> bool {
    let is_self_parent_or_static = match cname {
        ast::ClassId_::CIexpr(ast::Expr(_, _, ast::Expr_::Id(id))) => {
            string_utils::is_self(&id.1)
                || string_utils::is_parent(&id.1)
                || string_utils::is_static(&id.1)
        }
        _ => false,
    };
    string_utils::is_class(id) && !is_self_parent_or_static
}

fn emit_elem<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    elem: Option<&ast::Expr>,
    local_temp_kind: Option<StoredValueKind>,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq<'arena>, StackIndex)> {
    let alloc = env.arena;
    Ok(match elem {
        None => (instr::empty(alloc), 0),
        Some(expr) if expr.2.is_int() || expr.2.is_string() => (instr::empty(alloc), 0),
        Some(expr) => match &expr.2 {
            ast::Expr_::Lvar(x) if !is_local_this(env, &x.1) => {
                if local_temp_kind.is_some() {
                    (
                        instr::cgetquietl(
                            alloc,
                            get_local(e, env, &x.0, local_id::get_name(&x.1))?,
                        ),
                        0,
                    )
                } else if null_coalesce_assignment {
                    (
                        instr::cgetl(alloc, get_local(e, env, &x.0, local_id::get_name(&x.1))?),
                        1,
                    )
                } else {
                    (instr::empty(alloc), 0)
                }
            }
            ast::Expr_::ClassConst(x)
                if is_special_class_constant_accessed_with_class_id(&(x.0).2, &(x.1).1) =>
            {
                (instr::empty(alloc), 0)
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
    use ast::Expr as E;
    use ast::Expr_ as E_;
    let alloc = env.arena;
    match elem {
        // ELement missing (so it's array append)
        None => Ok((MemberKey::W, instr::empty(alloc))),
        Some(elem_expr) => match &elem_expr.2 {
            // Special case for local
            E_::Lvar(x) if !is_local_this(env, &x.1) => Ok((
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
                instr::empty(alloc),
            )),
            // Special case for literal integer
            E_::Int(s) => match ast_constant_folder::expr_to_typed_value(alloc, e, elem_expr) {
                Ok(TypedValue::Int(i)) => {
                    Ok((MemberKey::EI(i, ReadonlyOp::Any), instr::empty(alloc)))
                }
                _ => Err(Unrecoverable(format!("{} is not a valid integer index", s))),
            },
            // Special case for literal string
            E_::String(s) => {
                // FIXME: This is not safe--string literals are binary strings.
                // There's no guarantee that they're valid UTF-8.
                let s = unsafe { std::str::from_utf8_unchecked(s.as_slice()) };
                let s = bumpalo::collections::String::from_str_in(s, alloc).into_bump_str();
                Ok((
                    MemberKey::ET(Str::from(s), ReadonlyOp::Any),
                    instr::empty(alloc),
                ))
            }
            // Special case for class name
            E_::ClassConst(x)
                if is_special_class_constant_accessed_with_class_id(&(x.0).2, &(x.1).1) =>
            {
                let cname = match (&(x.0).2, env.scope.get_class()) {
                    (CI_::CIself, Some(cd)) => string_utils::strip_global_ns(cd.get_name_str()),
                    (CI_::CIexpr(E(_, _, E_::Id(id))), _) => string_utils::strip_global_ns(&id.1),
                    (CI_::CI(id), _) => string_utils::strip_global_ns(&id.1),
                    _ => {
                        return Err(Unrecoverable(
                            "Unreachable due to is_special_class_constant_accessed_with_class_id"
                                .into(),
                        ));
                    }
                };
                let fq_id =
                    class::ClassType::<'arena>::from_ast_name(alloc, &cname).to_raw_string();
                if e.options().emit_class_pointers() > 0 {
                    Ok((
                        MemberKey::ET(Str::from(fq_id), ReadonlyOp::Any),
                        instr::raise_class_string_conversion_warning(alloc),
                    ))
                } else {
                    Ok((
                        MemberKey::ET(Str::from(fq_id), ReadonlyOp::Any),
                        instr::empty(alloc),
                    ))
                }
            }
            _ => {
                // General case
                Ok((
                    MemberKey::EC(stack_index, ReadonlyOp::Any),
                    instr::empty(alloc),
                ))
            }
        },
    }
}

fn emit_store_for_simple_base<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    elem_stack_size: isize,
    base: &ast::Expr,
    local: Local<'arena>,
    is_base: bool,
    readonly_op: ReadonlyOp,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let (base_expr_instrs_begin, base_expr_instrs_end, base_setup_instrs, _, _) = emit_base(
        e,
        env,
        base,
        MemberOpMode::Define,
        false,
        BareThisOp::Notice,
        false,
        elem_stack_size,
        0,
        readonly_op,
    )?;
    let memberkey = MemberKey::EL(local, ReadonlyOp::Any);
    Ok(InstrSeq::gather(
        alloc,
        vec![
            base_expr_instrs_begin,
            base_expr_instrs_end,
            emit_pos(alloc, pos),
            base_setup_instrs,
            if is_base {
                instr::dim(alloc, MemberOpMode::Define, memberkey)
            } else {
                instr::setm(alloc, 0, memberkey)
            },
        ],
    ))
}

fn get_querym_op_mode(query_op: &QueryOp) -> MemberOpMode {
    match query_op {
        QueryOp::InOut => MemberOpMode::InOut,
        QueryOp::CGet => MemberOpMode::Warn,
        _ => MemberOpMode::ModeNone,
    }
}

fn emit_class_get<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    query_op: QueryOp,
    cid: &ast::ClassId,
    prop: &ast::ClassGetExpr,
    readonly_op: ReadonlyOp,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
    Ok(InstrSeq::gather(
        alloc,
        vec![
            InstrSeq::from((alloc, emit_class_expr(e, env, cexpr, prop)?)),
            match query_op {
                QueryOp::CGet => instr::cgets(alloc, readonly_op),
                QueryOp::Isset => instr::issets(alloc),
                QueryOp::CGetQuiet => {
                    return Err(Unrecoverable("emit_class_get: CGetQuiet".into()));
                }
                QueryOp::InOut => return Err(Unrecoverable("emit_class_get: InOut".into())),
            },
        ],
    ))
}

fn emit_conditional_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    etest: &ast::Expr,
    etrue: &Option<ast::Expr>,
    efalse: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    Ok(match etrue.as_ref() {
        Some(etrue) => {
            let false_label = e.label_gen_mut().next_regular();
            let end_label = e.label_gen_mut().next_regular();
            let r = emit_jmpz(e, env, etest, false_label)?;
            // only emit false branch if false_label is used
            let false_branch = if r.is_label_used {
                InstrSeq::gather(
                    alloc,
                    vec![instr::label(alloc, false_label), emit_expr(e, env, efalse)?],
                )
            } else {
                instr::empty(alloc)
            };
            // only emit true branch if there is fallthrough from condition
            let true_branch = if r.is_fallthrough {
                InstrSeq::gather(
                    alloc,
                    vec![
                        emit_expr(e, env, etrue)?,
                        emit_pos(alloc, pos),
                        instr::jmp(alloc, end_label),
                    ],
                )
            } else {
                instr::empty(alloc)
            };
            InstrSeq::gather(
                alloc,
                vec![
                    r.instrs,
                    true_branch,
                    false_branch,
                    // end_label is used to jump out of true branch so they should be emitted together
                    if r.is_fallthrough {
                        instr::label(alloc, end_label)
                    } else {
                        instr::empty(alloc)
                    },
                ],
            )
        }
        None => {
            let end_label = e.label_gen_mut().next_regular();
            let efalse_instr = emit_expr(e, env, efalse)?;
            let etest_instr = emit_expr(e, env, etest)?;
            InstrSeq::gather(
                alloc,
                vec![
                    etest_instr,
                    instr::dup(alloc),
                    instr::jmpnz(alloc, end_label),
                    instr::popc(alloc),
                    efalse_instr,
                    instr::label(alloc, end_label),
                ],
            )
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
        Ok(InstrSeq::gather(
            alloc,
            vec![
                instr::string(alloc, string_utils::locals::strip_dollar(id_name)),
                emit_pos(alloc, pos),
                instr::cgetg(alloc),
            ],
        ))
    } else {
        let local = get_local(e, env, pos, id_name)?;
        Ok(
            if is_local_this(env, id) && !env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) {
                emit_pos_then(alloc, pos, instr::barethis(alloc, notice))
            } else {
                instr::cgetl(alloc, local)
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
    let mut cexpr = ClassExpr::class_id_to_class_expr(e, false, true, &env.scope, cid);
    if let ClassExpr::Id(ast_defs::Id(_, name)) = &cexpr {
        if let Some(reified_var_cexpr) = get_reified_var_cexpr(env, pos, &name)? {
            cexpr = reified_var_cexpr;
        }
    }
    match cexpr {
        ClassExpr::Id(ast_defs::Id(pos, name)) => {
            let cid = class::ClassType::from_ast_name_and_mangle(alloc, &name);
            let cname = cid.to_raw_string();
            Ok(if string_utils::is_class(&id.1) {
                if e.options().emit_class_pointers() == 1 {
                    emit_pos_then(alloc, &pos, instr::resolveclass(alloc, cid))
                } else if e.options().emit_class_pointers() == 2 {
                    emit_pos_then(alloc, &pos, instr::lazyclass(alloc, cid))
                } else {
                    emit_pos_then(alloc, &pos, instr::string(alloc, cname))
                }
            } else {
                emit_symbol_refs::add_class(e, cid.clone());
                // TODO(hrust) enabel `let const_id = r#const::ConstType::from_ast_name(&id.1);`,
                // `from_ast_name` should be able to accpet Cow<str>
                let const_id =
                    r#const::ConstType(Str::new_str(alloc, string_utils::strip_global_ns(&id.1)));
                emit_pos_then(alloc, &pos, instr::clscnsd(alloc, const_id, cid))
            })
        }
        _ => {
            let load_const = if string_utils::is_class(&id.1) {
                if e.options().emit_class_pointers() == 2 {
                    instr::lazyclassfromclass(alloc)
                } else {
                    instr::classname(alloc)
                }
            } else {
                // TODO(hrust) enabel `let const_id = r#const::ConstType::from_ast_name(&id.1);`,
                // `from_ast_name` should be able to accpet Cow<str>
                let const_id =
                    r#const::ConstType(Str::new_str(alloc, string_utils::strip_global_ns(&id.1)));
                instr::clscns(alloc, const_id)
            };
            if string_utils::is_class(&id.1) && e.options().emit_class_pointers() == 1 {
                emit_load_class_ref(e, env, pos, cexpr)
            } else {
                Ok(InstrSeq::gather(
                    alloc,
                    vec![emit_load_class_ref(e, env, pos, cexpr)?, load_const],
                ))
            }
        }
    }
}

fn emit_unop<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    (uop, expr): &(ast_defs::Uop, ast::Expr),
) -> Result<InstrSeq<'arena>> {
    use ast_defs::Uop as U;
    let alloc = env.arena;
    match uop {
        U::Utild | U::Unot => Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_expr(e, env, expr)?,
                emit_pos_then(alloc, pos, from_unop(alloc, e.options(), uop)?),
            ],
        )),
        U::Uplus | U::Uminus => Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_pos(alloc, pos),
                instr::int(alloc, 0),
                emit_expr(e, env, expr)?,
                emit_pos_then(alloc, pos, from_unop(alloc, e.options(), uop)?),
            ],
        )),
        U::Uincr | U::Udecr | U::Upincr | U::Updecr => emit_lval_op(
            e,
            env,
            pos,
            LValOp::IncDec(unop_to_incdec_op(e.options(), uop)?),
            expr,
            None,
            false,
        ),
        U::Usilence => e.local_scope(|e| {
            let temp_local = e.local_gen_mut().get_unnamed();
            Ok(InstrSeq::gather(
                alloc,
                vec![
                    emit_pos(alloc, pos),
                    instr::silence_start(alloc, temp_local),
                    {
                        let try_instrs = emit_expr(e, env, expr)?;
                        let catch_instrs = InstrSeq::gather(
                            alloc,
                            vec![emit_pos(alloc, pos), instr::silence_end(alloc, temp_local)],
                        );
                        InstrSeq::create_try_catch(
                            alloc,
                            e.label_gen_mut(),
                            None,
                            false, /* skip_throw */
                            try_instrs,
                            catch_instrs,
                        )
                    },
                    emit_pos(alloc, pos),
                    instr::silence_end(alloc, temp_local),
                ],
            ))
        }),
    }
}

fn unop_to_incdec_op(opts: &Options, op: &ast_defs::Uop) -> Result<IncdecOp> {
    let if_check_or = |op1, op2| Ok(if opts.check_int_overflow() { op1 } else { op2 });
    use {ast_defs::Uop as U, IncdecOp as I};
    match op {
        U::Uincr => if_check_or(I::PreIncO, I::PreInc),
        U::Udecr => if_check_or(I::PreDecO, I::PreDec),
        U::Upincr => if_check_or(I::PostIncO, I::PostInc),
        U::Updecr => if_check_or(I::PostDecO, I::PostDec),
        _ => Err(Unrecoverable("invalid incdec op".into())),
    }
}

fn from_unop<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    op: &ast_defs::Uop,
) -> Result<InstrSeq<'arena>> {
    use ast_defs::Uop as U;
    Ok(match op {
        U::Utild => instr::bitnot(alloc),
        U::Unot => instr::not(alloc),
        U::Uplus => {
            if opts.check_int_overflow() {
                instr::addo(alloc)
            } else {
                instr::add(alloc)
            }
        }
        U::Uminus => {
            if opts.check_int_overflow() {
                instr::subo(alloc)
            } else {
                instr::sub(alloc)
            }
        }
        _ => {
            return Err(Unrecoverable(
                "this unary operation cannot be translated".into(),
            ));
        }
    })
}

fn binop_to_eqop(opts: &Options, op: &ast_defs::Bop) -> Option<EqOp> {
    use {ast_defs::Bop as B, EqOp::*};
    match op {
        B::Plus => Some(if opts.check_int_overflow() {
            PlusEqualO
        } else {
            PlusEqual
        }),
        B::Minus => Some(if opts.check_int_overflow() {
            MinusEqualO
        } else {
            MinusEqual
        }),
        B::Star => Some(if opts.check_int_overflow() {
            MulEqualO
        } else {
            MulEqual
        }),
        B::Slash => Some(DivEqual),
        B::Starstar => Some(PowEqual),
        B::Amp => Some(AndEqual),
        B::Bar => Some(OrEqual),
        B::Xor => Some(XorEqual),
        B::Ltlt => Some(SlEqual),
        B::Gtgt => Some(SrEqual),
        B::Percent => Some(ModEqual),
        B::Dot => Some(ConcatEqual),
        _ => None,
    }
}

#[allow(clippy::needless_lifetimes)]
fn optimize_null_checks<'arena, 'decl>(e: &Emitter<'arena, 'decl>) -> bool {
    e.options()
        .hack_compiler_flags
        .contains(CompilerFlags::OPTIMIZE_NULL_CHECKS)
}

fn from_binop<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    op: &ast_defs::Bop,
) -> Result<InstrSeq<'arena>> {
    use ast_defs::Bop as B;
    Ok(match op {
        B::Plus => {
            if opts.check_int_overflow() {
                instr::addo(alloc)
            } else {
                instr::add(alloc)
            }
        }
        B::Minus => {
            if opts.check_int_overflow() {
                instr::subo(alloc)
            } else {
                instr::sub(alloc)
            }
        }
        B::Star => {
            if opts.check_int_overflow() {
                instr::mulo(alloc)
            } else {
                instr::mul(alloc)
            }
        }
        B::Slash => instr::div(alloc),
        B::Eqeq => instr::eq(alloc),
        B::Eqeqeq => instr::same(alloc),
        B::Starstar => instr::pow(alloc),
        B::Diff => instr::neq(alloc),
        B::Diff2 => instr::nsame(alloc),
        B::Lt => instr::lt(alloc),
        B::Lte => instr::lte(alloc),
        B::Gt => instr::gt(alloc),
        B::Gte => instr::gte(alloc),
        B::Dot => instr::concat(alloc),
        B::Amp => instr::bitand(alloc),
        B::Bar => instr::bitor(alloc),
        B::Ltlt => instr::shl(alloc),
        B::Gtgt => instr::shr(alloc),
        B::Cmp => instr::cmp(alloc),
        B::Percent => instr::mod_(alloc),
        B::Xor => instr::bitxor(alloc),
        B::Eq(_) => return Err(Unrecoverable("assignment is emitted differently".into())),
        B::QuestionQuestion => {
            return Err(Unrecoverable(
                "null coalescence is emitted differently".into(),
            ));
        }
        B::Barbar | B::Ampamp => {
            return Err(Unrecoverable(
                "short-circuiting operator cannot be generated as a simple binop".into(),
            ));
        }
    })
}

fn emit_first_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<(InstrSeq<'arena>, bool)> {
    let alloc = env.arena;
    Ok(match &expr.2 {
        ast::Expr_::Lvar(l)
            if !((is_local_this(env, &l.1) && !env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS))
                || superglobals::is_any_global(local_id::get_name(&l.1))) =>
        {
            (
                instr::cgetl2(alloc, get_local(e, env, &l.0, local_id::get_name(&l.1))?),
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
    let alloc = env.arena;
    let (instrs1, is_under_top) = emit_first_expr(e, env, e1)?;
    let instrs2 = emit_expr(e, env, e2)?;
    let instrs2_is_var = e2.2.is_lvar();
    Ok(InstrSeq::gather(
        alloc,
        if is_under_top {
            if instrs2_is_var {
                vec![emit_pos(alloc, outer_pos), instrs2, instrs1]
            } else {
                vec![instrs2, emit_pos(alloc, outer_pos), instrs1]
            }
        } else if instrs2_is_var {
            vec![instrs1, emit_pos(alloc, outer_pos), instrs2]
        } else {
            vec![instrs1, instrs2, emit_pos(alloc, outer_pos)]
        },
    ))
}

fn emit_readonly_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &aast::Expr<(), ()>,
) -> Result<InstrSeq<'arena>> {
    match &expr.2 {
        aast::Expr_::ObjGet(x) => {
            Ok(emit_obj_get(e, env, pos, QueryOp::CGet, &x.0, &x.1, &x.2, false, true)?.0)
        }
        aast::Expr_::Call(c) => emit_call_expr(e, env, pos, None, true, c),
        aast::Expr_::ClassGet(x) => {
            emit_class_get(e, env, QueryOp::CGet, &x.0, &x.1, ReadonlyOp::Any)
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
) -> Result<(InstrSeq<'arena>, Option<NumParams>)> {
    let alloc = env.arena;
    match &expr.2 {
        ast::Expr_::Lvar(lid) if !is_local_this(env, &lid.1) => Ok((
            instr::cgetquietl(alloc, get_local(e, env, pos, local_id::get_name(&lid.1))?),
            None,
        )),
        ast::Expr_::ArrayGet(x) => emit_array_get(
            e,
            env,
            pos,
            None,
            QueryOp::CGetQuiet,
            &x.0,
            x.1.as_ref(),
            false,
            null_coalesce_assignment,
        ),
        ast::Expr_::ObjGet(x) if x.as_ref().3 == ast::PropOrMethod::IsProp => emit_obj_get(
            e,
            env,
            pos,
            QueryOp::CGetQuiet,
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
    let alloc = env.arena;
    let end_label = e.label_gen_mut().next_regular();
    let do_set_label = e.label_gen_mut().next_regular();
    let l_nonnull = e.local_gen_mut().get_unnamed();
    let (quiet_instr, querym_n_unpopped) = emit_quiet_expr(e, env, pos, e1, true)?;
    let emit_popc_n = |n_unpopped| match n_unpopped {
        Some(n) => InstrSeq::gather(
            alloc,
            iter::repeat_with(|| instr::popc(alloc))
                .take(n)
                .collect::<Vec<_>>(),
        ),
        None => instr::empty(alloc),
    };
    Ok(InstrSeq::gather(
        alloc,
        vec![
            quiet_instr,
            instr::dup(alloc),
            instr::istypec(alloc, IstypeOp::OpNull),
            instr::jmpnz(alloc, do_set_label),
            instr::popl(alloc, l_nonnull),
            emit_popc_n(querym_n_unpopped),
            instr::pushl(alloc, l_nonnull),
            instr::jmp(alloc, end_label),
            instr::label(alloc, do_set_label),
            instr::popc(alloc),
            emit_lval_op(e, env, pos, LValOp::Set, e1, Some(e2), true)?,
            instr::label(alloc, end_label),
        ],
    ))
}

fn emit_short_circuit_op<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let its_true = e.label_gen_mut().next_regular();
    let its_done = e.label_gen_mut().next_regular();
    let jmp_instrs = emit_jmpnz(e, env, expr, its_true)?;
    Ok(if jmp_instrs.is_fallthrough {
        InstrSeq::gather(
            alloc,
            vec![
                jmp_instrs.instrs,
                emit_pos(alloc, pos),
                instr::false_(alloc),
                instr::jmp(alloc, its_done),
                if jmp_instrs.is_label_used {
                    InstrSeq::gather(
                        alloc,
                        vec![
                            instr::label(alloc, its_true),
                            emit_pos(alloc, pos),
                            instr::true_(alloc),
                        ],
                    )
                } else {
                    instr::empty(alloc)
                },
                instr::label(alloc, its_done),
            ],
        )
    } else {
        InstrSeq::gather(
            alloc,
            vec![
                jmp_instrs.instrs,
                if jmp_instrs.is_label_used {
                    InstrSeq::gather(
                        alloc,
                        vec![
                            instr::label(alloc, its_true),
                            emit_pos(alloc, pos),
                            instr::true_(alloc),
                        ],
                    )
                } else {
                    instr::empty(alloc)
                },
            ],
        )
    })
}

fn emit_binop<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let (op, e1, e2) = expr.2.as_binop().unwrap();
    use ast_defs::Bop as B;
    match op {
        B::Ampamp | B::Barbar => emit_short_circuit_op(e, env, pos, &expr),
        B::Eq(None) => emit_lval_op(e, env, pos, LValOp::Set, e1, Some(e2), false),
        B::Eq(Some(eop)) if eop.is_question_question() => {
            emit_null_coalesce_assignment(e, env, pos, e1, e2)
        }
        B::Eq(Some(eop)) => match binop_to_eqop(e.options(), eop) {
            None => Err(Unrecoverable("illegal eq op".into())),
            Some(op) => emit_lval_op(e, env, pos, LValOp::SetOp(op), e1, Some(e2), false),
        },
        B::QuestionQuestion => {
            let end_label = e.label_gen_mut().next_regular();
            let rhs = emit_expr(e, env, e2)?;
            Ok(InstrSeq::gather(
                alloc,
                vec![
                    emit_quiet_expr(e, env, pos, e1, false)?.0,
                    instr::dup(alloc),
                    instr::istypec(alloc, IstypeOp::OpNull),
                    instr::not(alloc),
                    instr::jmpnz(alloc, end_label),
                    instr::popc(alloc),
                    rhs,
                    instr::label(alloc, end_label),
                ],
            ))
        }
        _ => {
            let default = |e: &mut Emitter<'arena, 'decl>| {
                Ok(InstrSeq::gather(
                    alloc,
                    vec![
                        emit_two_exprs(e, env, pos, e1, e2)?,
                        from_binop(alloc, e.options(), op)?,
                    ],
                ))
            };
            if optimize_null_checks(e) {
                match op {
                    B::Eqeqeq if e2.2.is_null() => emit_is_null(e, env, e1),
                    B::Eqeqeq if e1.2.is_null() => emit_is_null(e, env, e2),
                    B::Diff2 if e2.2.is_null() => Ok(InstrSeq::gather(
                        alloc,
                        vec![emit_is_null(e, env, e1)?, instr::not(alloc)],
                    )),
                    B::Diff2 if e1.2.is_null() => Ok(InstrSeq::gather(
                        alloc,
                        vec![emit_is_null(e, env, e2)?, instr::not(alloc)],
                    )),
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
        let alloc = e.alloc;
        let mut pipe_env = env.clone();
        pipe_env.with_pipe_var(local);
        let rhs_instrs = emit_expr(e, &pipe_env, e2)?;
        Ok((
            InstrSeq::gather(alloc, vec![lhs_instrs, instr::popl(alloc, local)]),
            rhs_instrs,
            instr::unsetl(alloc, local),
        ))
    })
}

fn emit_as<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    (expr, h, is_nullable): &(ast::Expr, aast_defs::Hint, bool),
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    e.local_scope(|e| {
        let arg_local = e.local_gen_mut().get_unnamed();
        let type_struct_local = e.local_gen_mut().get_unnamed();
        let (ts_instrs, is_static) = emit_reified_arg(e, env, pos, true, h)?;
        let then_label = e.label_gen_mut().next_regular();
        let done_label = e.label_gen_mut().next_regular();
        let main_block = |ts_instrs, resolve| {
            InstrSeq::gather(
                alloc,
                vec![
                    ts_instrs,
                    instr::setl(alloc, type_struct_local),
                    match resolve {
                        TypestructResolveOp::Resolve => instr::is_type_structc_resolve(alloc),
                        TypestructResolveOp::DontResolve => {
                            instr::is_type_structc_dontresolve(alloc)
                        }
                    },
                    instr::jmpnz(alloc, then_label),
                    if *is_nullable {
                        InstrSeq::gather(
                            alloc,
                            vec![instr::null(alloc), instr::jmp(alloc, done_label)],
                        )
                    } else {
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::pushl(alloc, arg_local),
                                instr::pushl(alloc, type_struct_local),
                                instr::throwastypestructexception(alloc),
                            ],
                        )
                    },
                ],
            )
        };
        let i2 = if is_static {
            main_block(
                get_type_structure_for_hint(e, &[], &IndexSet::new(), h)?,
                TypestructResolveOp::Resolve,
            )
        } else {
            main_block(ts_instrs, TypestructResolveOp::DontResolve)
        };
        let i1 = emit_expr(e, env, expr)?;
        Ok(InstrSeq::gather(
            alloc,
            vec![
                i1,
                instr::setl(alloc, arg_local),
                i2,
                instr::label(alloc, then_label),
                instr::pushl(alloc, arg_local),
                instr::unsetl(alloc, type_struct_local),
                instr::label(alloc, done_label),
            ],
        ))
    })
}

fn emit_cast<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    hint: &aast_defs::Hint_,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    use aast_defs::Hint_ as H_;
    let op = match hint {
        H_::Happly(ast_defs::Id(_, id), hints) if hints.is_empty() => {
            let id = string_utils::strip_ns(id);
            match string_utils::strip_hh_ns(&id).as_ref() {
                typehints::INT => instr::cast_int(alloc),
                typehints::BOOL => instr::cast_bool(alloc),
                typehints::STRING => instr::cast_string(alloc),
                typehints::FLOAT => instr::cast_double(alloc),
                _ => {
                    return Err(emit_fatal::raise_fatal_parse(
                        pos,
                        format!("Invalid cast type: {}", id),
                    ));
                }
            }
        }
        _ => return Err(emit_fatal::raise_fatal_parse(pos, "Invalid cast type")),
    };
    Ok(InstrSeq::gather(
        alloc,
        vec![emit_expr(e, env, expr)?, emit_pos(alloc, pos), op],
    ))
}

pub fn emit_unset_expr<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    emit_lval_op_nonlist(
        e,
        env,
        &expr.1,
        LValOp::Unset,
        expr,
        instr::empty(alloc),
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
    kind: Setrange,
    args: &[(ParamKind, ast::Expr)],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let raise_fatal = |msg: &str| {
        Err(emit_fatal::raise_fatal_parse(
            pos,
            format!("{} {}", name, msg),
        ))
    };

    // TODO(hgoldstein) Weirdly enough, we *ignore* when the first argument is `inout`
    // unconditionally. We probably want to either _always_ require it or never require it.
    let ((_, base), (pk_offset, offset), (pk_src, src), args) = if args.len() >= 3 {
        (&args[0], &args[1], &args[2], &args[3..])
    } else {
        return raise_fatal("expects at least 3 arguments");
    };

    ensure_normal_paramkind(pk_offset)?;
    ensure_normal_paramkind(pk_src)?;

    let count_instrs = match (args, kind.vec) {
        ([c], true) => emit_expr(e, env, expect_normal_paramkind(c)?)?,
        ([], _) => instr::int(alloc, -1),
        (_, false) => return raise_fatal("expects no more than 3 arguments"),
        (_, true) => return raise_fatal("expects no more than 4 arguments"),
    };

    let (base_expr, cls_expr, base_setup, base_stack, cls_stack) = emit_base(
        e,
        env,
        base,
        MemberOpMode::Define,
        false, /* is_object */
        BareThisOp::Notice,
        false,           /*null_coalesce_assignment*/
        3,               /* base_offset */
        3,               /* rhs_stack_size */
        ReadonlyOp::Any, /* readonly_op */
    )?;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            base_expr,
            cls_expr,
            emit_expr(e, env, offset)?,
            emit_expr(e, env, src)?,
            count_instrs,
            base_setup,
            instr::instr(
                alloc,
                Instruct::IFinal(InstructFinal::SetRangeM(
                    (base_stack + cls_stack)
                        .try_into()
                        .expect("StackIndex overflow"),
                    kind.size.try_into().expect("Setrange size overflow"),
                    kind.op,
                )),
            ),
        ],
    ))
}

pub fn is_reified_tparam<'a, 'arena>(
    env: &Env<'a, 'arena>,
    is_fun: bool,
    name: &str,
) -> Option<(usize, bool)> {
    let is = |tparams: &[ast::Tparam]| {
        let is_soft = |ual: &Vec<ast::UserAttribute>| {
            ual.iter().any(|ua| user_attributes::is_soft(&ua.name.1))
        };
        use ast::ReifyKind::*;
        tparams.iter().enumerate().find_map(|(i, tp)| {
            if (tp.reified == Reified || tp.reified == SoftReified) && tp.name.1 == name {
                Some((i, is_soft(&tp.user_attributes)))
            } else {
                None
            }
        })
    };
    if is_fun {
        is(env.scope.get_fun_tparams())
    } else {
        is(&env.scope.get_class_tparams()[..])
    }
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
///   AddO
///   # Section 2, constructing the base address of $arr[3]
///   BaseL $arr Warn
///   Dim Warn EI:3
///   # Section 3, indexing the array using the value at stack position 0 (EC:0)
///   QueryM 1 CGet EC:0
///
fn emit_base<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    mode: MemberOpMode,
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
            i.base_stack_size as isize,
            i.cls_stack_size as isize,
        )),
        ArrayGetBase::Inout { .. } => Err(unrecoverable("unexpected input")),
    }
}

fn is_trivial(env: &Env<'_, '_>, is_base: bool, expr: &ast::Expr) -> bool {
    use ast::Expr_ as E_;
    match &expr.2 {
        E_::Int(_) | E_::String(_) => true,
        E_::Lvar(x) => !is_local_this(env, &x.1) || env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS),
        E_::ArrayGet(_) if !is_base => false,
        E_::ArrayGet(x) => {
            is_trivial(env, is_base, &x.0)
                && (x.1)
                    .as_ref()
                    .map_or(true, |e| is_trivial(env, is_base, &e))
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
    mode: MemberOpMode,
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
    let base_mode = if mode == MemberOpMode::InOut {
        MemberOpMode::Warn
    } else {
        mode
    };
    let local_temp_kind = get_local_temp_kind(env, true, inout_param_info, Some(expr));
    let emit_default = |
        e: &mut Emitter<'arena, 'decl>,
        base_instrs,
        cls_instrs,
        setup_instrs,
        base_stack_size,
        cls_stack_size,
    | {
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
                    store: instr::basel(alloc, local, MemberOpMode::Define, ReadonlyOp::Any),
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
    // Called when emitting a base with MemberOpMode::Define on a Readonly expression
    let emit_readonly_lval_base = |
        e: &mut Emitter<'arena, 'decl>,
        env: &Env<'a, 'arena>,
        inner_expr: &ast::Expr, // expression inside of readonly expression
    | -> Option<Result<ArrayGetBase<'_>>> {
        // Readonly local variable requires a CheckROCOW
        if let aast::Expr(_, _, E_::Lvar(x)) = &*inner_expr {
            if !is_local_this(env, &x.1) || env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) {
                match get_local(e, env, &x.0, &(x.1).1) {
                    Ok(v) => {
                    let base_instr = if local_temp_kind.is_some() {
                        instr::cgetquietl(alloc, v)
                    } else {
                        instr::empty(alloc)
                    };
                    Some(Ok(emit_default(
                        e,
                        base_instr,
                        instr::empty(alloc),
                        instr::basel(alloc, v, MemberOpMode::Define, ReadonlyOp::CheckROCOW),
                        0,
                        0,
                    )))
                 }
                    Err(e) => Some(Err(e))
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
                instr::empty(alloc),
                emit_pos_then(alloc, pos, instr::basec(alloc, base_offset, base_mode)),
                1,
                0,
            ))
        };

    use ast::Expr_ as E_;
    match expr_ {
        // Readonly expression in assignment
        E_::ReadonlyExpr(r) if base_mode == MemberOpMode::Define => {
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
        E_::Lvar(x) if superglobals::is_superglobal(&(x.1).1) => {
            let base_instrs = emit_pos_then(
                alloc,
                &x.0,
                instr::string(alloc, string_utils::locals::strip_dollar(&(x.1).1)),
            );

            Ok(emit_default(
                e,
                base_instrs,
                instr::empty(alloc),
                instr::basegc(alloc, base_offset, base_mode),
                1,
                0,
            ))
        }
        E_::Lvar(x) if is_object && (x.1).1 == special_idents::THIS => {
            let base_instrs = emit_pos_then(alloc, &x.0, instr::checkthis(alloc));
            Ok(emit_default(
                e,
                base_instrs,
                instr::empty(alloc),
                instr::baseh(alloc),
                0,
                0,
            ))
        }
        E_::Lvar(x)
            if !is_local_this(env, &x.1) || env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) =>
        {
            let v = get_local(e, env, &x.0, &(x.1).1)?;
            let base_instr = if local_temp_kind.is_some() {
                instr::cgetquietl(alloc, v)
            } else {
                instr::empty(alloc)
            };
            Ok(emit_default(
                e,
                base_instr,
                instr::empty(alloc),
                instr::basel(alloc, v, base_mode, ReadonlyOp::Any),
                0,
                0,
            ))
        }
        E_::Lvar(lid) => {
            let local = emit_local(e, env, notice, lid)?;
            Ok(emit_default(
                e,
                local,
                instr::empty(alloc),
                instr::basec(alloc, base_offset, base_mode),
                1,
                0,
            ))
        }
        E_::ArrayGet(x) => match (&(x.0).1, x.1.as_ref()) {
            // $a[] can not be used as the base of an array get unless as an lval
            (_, None)
                if !env
                    .flags
                    .contains(hhbc_by_ref_env::Flags::ALLOWS_ARRAY_APPEND) =>
            {
                Err(emit_fatal::raise_fatal_runtime(
                    pos,
                    "Can't use [] for reading",
                ))
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
                    InstrSeq::gather(
                        alloc,
                        vec![warninstr, base_setup_instrs, instr::dim(alloc, mode, mk)],
                    )
                };
                Ok(match (base_result, local_temp_kind) {
                    // both base and index don't use temps - fallback to default handler
                    (ArrayGetBase::Regular(base), None) => emit_default(
                        e,
                        InstrSeq::gather(alloc, vec![base.base_instrs, elem_instrs]),
                        base.cls_instrs,
                        make_setup_instrs(base.setup_instrs),
                        base.base_stack_size + elem_stack_size,
                        base.cls_stack_size,
                    ),
                    // base does not need temps but index does
                    (ArrayGetBase::Regular(base), Some(local_temp)) => {
                        let local = e.local_gen_mut().get_unnamed();
                        let base_instrs =
                            InstrSeq::gather(alloc, vec![base.base_instrs, elem_instrs]);
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
                            store: InstrSeq::gather(
                                alloc,
                                vec![store, instr::dim(alloc, MemberOpMode::Define, mk)],
                            ),
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
                            store: InstrSeq::gather(
                                alloc,
                                vec![
                                    store,
                                    instr::dim(
                                        alloc,
                                        MemberOpMode::Define,
                                        MemberKey::EL(local, ReadonlyOp::Any),
                                    ),
                                ],
                            ),
                        }
                    }
                })
            }
        },
        E_::ObjGet(x) if x.as_ref().3 == ast::PropOrMethod::IsProp => {
            let (base_expr, prop_expr, null_flavor, _) = &**x;
            Ok(match prop_expr.2.as_id() {
                Some(ast_defs::Id(_, s)) if string_utils::is_xhp(&s) => {
                    let base_instrs = emit_xhp_obj_get(e, env, pos, base_expr, &s, null_flavor)?;
                    emit_default(
                        e,
                        base_instrs,
                        instr::empty(alloc),
                        instr::basec(alloc, base_offset, base_mode),
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
                    let final_instr = instr::dim(alloc, mode, mk);
                    emit_default(
                        e,
                        InstrSeq::gather(alloc, vec![base_expr_instrs_begin, prop_instrs]),
                        base_expr_instrs_end,
                        InstrSeq::gather(alloc, vec![base_setup_instrs, final_instr]),
                        total_stack_size,
                        cls_stack_size,
                    )
                }
            })
        }
        E_::ClassGet(x) => {
            let (cid, prop, _) = &**x;
            let cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
            let (cexpr_begin, cexpr_end) = emit_class_expr(e, env, cexpr, prop)?;
            Ok(emit_default(
                e,
                cexpr_begin,
                cexpr_end,
                instr::basesc(
                    alloc,
                    base_offset + 1,
                    rhs_stack_size,
                    base_mode,
                    readonly_op,
                ),
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
    let alloc = env.arena;
    exprs
        .iter()
        .map(|e| emit_ignored_expr(emitter, env, pos, e))
        .collect::<Result<Vec<_>>>()
        .map(|x| InstrSeq::gather(alloc, x))
}

// TODO(hrust): change pos from &Pos to Option<&Pos>, since Pos::make_none() still allocate mem.
pub fn emit_ignored_expr<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            emit_expr(emitter, env, expr)?,
            emit_pos_then(alloc, pos, instr::popc(alloc)),
        ],
    ))
}

pub fn emit_lval_op<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    pos: &Pos,
    op: LValOp,
    expr1: &ast::Expr,
    expr2: Option<&ast::Expr>,
    null_coalesce_assignment: bool,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match (op, &expr1.2, expr2) {
        (LValOp::Set, ast::Expr_::List(l), Some(expr2)) => {
            let instr_rhs = emit_expr(e, env, expr2)?;
            let has_elements = l.iter().any(|e| !e.2.is_omitted());
            if !has_elements {
                Ok(instr_rhs)
            } else {
                scope::with_unnamed_local(e, |e, local| {
                    let alloc = e.alloc;
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
                        InstrSeq::gather(
                            alloc,
                            vec![instr_lhs, instr_rhs, instr::popl(alloc, local)],
                        ),
                        instr_assign,
                        instr::pushl(alloc, local),
                    ))
                })
            }
        }
        _ => e.local_scope(|e| {
            let (rhs_instrs, rhs_stack_size, rhs_readonly) = match expr2 {
                None => (instr::empty(alloc), 0, false),
                Some(aast::Expr(_, _, aast::Expr_::Yield(af))) => {
                    let temp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![
                                emit_yield(e, env, pos, af)?,
                                instr::setl(alloc, temp),
                                instr::popc(alloc),
                                instr::pushl(alloc, temp),
                            ],
                        ),
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
    use aast::Expr_ as E_;
    Ok(match expr {
        E_::Call(c)
            if ((c.0).2)
                .as_id()
                .map_or(false, |id| id.1 == special_functions::ECHO) =>
        {
            false
        }
        E_::ObjGet(o) if o.as_ref().3 == ast::PropOrMethod::IsProp => true,
        E_::ClassGet(c) if c.as_ref().2 == ast::PropOrMethod::IsProp => true,
        E_::Lvar(_)
        | E_::ArrayGet(_)
        | E_::Call(_)
        | E_::FunctionPointer(_)
        | E_::New(_)
        | E_::Record(_)
        | E_::Yield(_)
        | E_::Cast(_)
        | E_::Eif(_)
        | E_::Tuple(_)
        | E_::Varray(_)
        | E_::Darray(_)
        | E_::Collection(_)
        | E_::Clone(_)
        | E_::Unop(_)
        | E_::As(_)
        | E_::Upcast(_)
        | E_::Await(_)
        | E_::ReadonlyExpr(_)
        | E_::ClassConst(_) => true,
        E_::Pipe(p) => can_use_as_rhs_in_list_assignment(&(p.2).2)?,
        E_::Binop(b) => {
            if let ast_defs::Bop::Eq(None) = &b.0 {
                if (b.1).2.is_list() {
                    return can_use_as_rhs_in_list_assignment(&(b.2).2);
                }
            }
            b.0.is_plus() || b.0.is_question_question() || b.0.is_any_eq()
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
fn emit_array_get_fixed<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    last_usage: bool,
    local: Local<'arena>,
    indices: &[isize],
) -> InstrSeq<'arena> {
    let (base, stack_count) = if last_usage {
        (
            InstrSeq::gather(
                alloc,
                vec![
                    instr::pushl(alloc, local),
                    instr::basec(alloc, 0, MemberOpMode::Warn),
                ],
            ),
            1,
        )
    } else {
        (
            instr::basel(alloc, local, MemberOpMode::Warn, ReadonlyOp::Any),
            0,
        )
    };
    let indices = InstrSeq::gather(
        alloc,
        indices
            .iter()
            .enumerate()
            .rev()
            .map(|(i, ix)| {
                let mk = MemberKey::EI(*ix as i64, ReadonlyOp::Any);
                if i == 0 {
                    instr::querym(alloc, stack_count, QueryOp::CGet, mk)
                } else {
                    instr::dim(alloc, MemberOpMode::Warn, mk)
                }
            })
            .collect(),
    );
    InstrSeq::gather(alloc, vec![base, indices])
}

// Generate code for each lvalue assignment in a list destructuring expression.
// Lvalues are assigned right-to-left, regardless of the nesting structure. So
//      list($a, list($b, $c)) = $d
//  and list(list($a, $b), $c) = $d
//  will both assign to $c, $b and $a in that order.
//  Returns a pair of instructions:
//  1. initialization part of the left hand side
//  2. assignment
//  this is necessary to handle cases like:
//  list($a[$f()]) = b();
//  here f() should be invoked before b()
pub fn emit_lval_op_list<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    outer_pos: &Pos,
    local: Option<&Local<'arena>>,
    indices: &[isize],
    expr: &ast::Expr,
    last_usage: bool,
    rhs_readonly: bool,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    use ast::Expr_ as E_;
    use hhbc_by_ref_options::Php7Flags;
    let alloc = env.arena;
    let is_ltr = e.options().php7_flags.contains(Php7Flags::LTR_ASSIGN);
    match &expr.2 {
        E_::List(exprs) => {
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
                InstrSeq::gather(alloc, lhs_instrs),
                InstrSeq::gather(
                    alloc,
                    if !is_ltr {
                        set_instrs.into_iter().rev().collect()
                    } else {
                        set_instrs
                    },
                ),
            ))
        }
        E_::Omitted => Ok((instr::empty(alloc), instr::empty(alloc))),
        _ => {
            // Generate code to access the element from the array
            let access_instrs = match (local, indices) {
                (Some(loc), [_, ..]) => {
                    emit_array_get_fixed(alloc, last_usage, loc.to_owned(), indices)
                }
                (Some(loc), []) => {
                    if last_usage {
                        instr::pushl(alloc, loc.to_owned())
                    } else {
                        instr::cgetl(alloc, loc.to_owned())
                    }
                }
                (None, _) => instr::null(alloc),
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
                    instr::empty(alloc),
                    InstrSeq::gather(
                        alloc,
                        vec![lhs_instrs, rhs_instrs, set_op, instr::popc(alloc)],
                    ),
                )
            } else {
                (
                    lhs_instrs,
                    InstrSeq::gather(
                        alloc,
                        vec![instr::empty(alloc), rhs_instrs, set_op, instr::popc(alloc)],
                    ),
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
    rhs_stack_size: isize,
    rhs_readonly: bool,
    null_coalesce_assignment: bool,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
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
    .map(|(lhs, rhs, setop)| InstrSeq::gather(alloc, vec![lhs, rhs, setop]))
}

pub fn emit_final_global_op<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    pos: &Pos,
    op: LValOp,
) -> InstrSeq<'arena> {
    use LValOp as L;
    match op {
        L::Set => emit_pos_then(alloc, pos, instr::setg(alloc)),
        L::SetOp(op) => instr::setopg(alloc, op),
        L::IncDec(op) => instr::incdecg(alloc, op),
        L::Unset => emit_pos_then(alloc, pos, instr::unsetg(alloc)),
    }
}

pub fn emit_final_local_op<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    pos: &Pos,
    op: LValOp,
    lid: Local<'arena>,
) -> InstrSeq<'arena> {
    use LValOp as L;
    emit_pos_then(
        alloc,
        pos,
        match op {
            L::Set => instr::setl(alloc, lid),
            L::SetOp(op) => instr::setopl(alloc, lid, op),
            L::IncDec(op) => instr::incdecl(alloc, lid, op),
            L::Unset => instr::unsetl(alloc, lid),
        },
    )
}

fn emit_final_member_op<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    stack_size: usize,
    op: LValOp,
    mk: MemberKey<'arena>,
) -> InstrSeq<'arena> {
    use LValOp as L;
    match op {
        L::Set => instr::setm(alloc, stack_size, mk),
        L::SetOp(op) => instr::setopm(alloc, stack_size, op, mk),
        L::IncDec(op) => instr::incdecm(alloc, stack_size, op, mk),
        L::Unset => instr::unsetm(alloc, stack_size, mk),
    }
}

fn emit_final_static_op<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    cid: &ast::ClassId,
    prop: &ast::ClassGetExpr,
    op: LValOp,
) -> Result<InstrSeq<'arena>> {
    use LValOp as L;
    Ok(match op {
        L::Set => instr::sets(alloc, ReadonlyOp::Any),
        L::SetOp(op) => instr::setops(alloc, op),
        L::IncDec(op) => instr::incdecs(alloc, op),
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
    rhs_stack_size: isize,
    rhs_readonly: bool,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>)> {
    let f = |alloc: &'arena bumpalo::Bump, env: &mut Env<'a, 'arena>| {
        use ast::Expr_ as E_;
        let pos = &expr.1;
        Ok(match &expr.2 {
            E_::Lvar(v) if superglobals::is_any_global(local_id::get_name(&v.1)) => (
                emit_pos_then(
                    alloc,
                    &v.0,
                    instr::string(alloc, string_utils::lstrip(local_id::get_name(&v.1), "$")),
                ),
                rhs_instrs,
                emit_final_global_op(alloc, outer_pos, op),
            ),
            E_::Lvar(v) if is_local_this(env, &v.1) && op.is_incdec() => (
                emit_local(e, env, BareThisOp::Notice, v)?,
                rhs_instrs,
                instr::empty(alloc),
            ),
            E_::Lvar(v) if !is_local_this(env, &v.1) || op == LValOp::Unset => {
                (instr::empty(alloc), rhs_instrs, {
                    let lid = get_local(e, env, &v.0, &(v.1).1)?;
                    emit_final_local_op(alloc, outer_pos, op, lid)
                })
            }
            E_::ArrayGet(x) => match (&(x.0).1, x.1.as_ref()) {
                (_, None)
                    if !env
                        .flags
                        .contains(hhbc_by_ref_env::Flags::ALLOWS_ARRAY_APPEND) =>
                {
                    return Err(emit_fatal::raise_fatal_runtime(
                        pos,
                        "Can't use [] for reading",
                    ));
                }
                (_, opt_elem_expr) => {
                    let mode = match op {
                        LValOp::Unset => MemberOpMode::Unset,
                        _ => MemberOpMode::Define,
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
                    let final_instr = emit_pos_then(
                        alloc,
                        pos,
                        emit_final_member_op(alloc, total_stack_size as usize, op, mk),
                    );
                    (
                        // Don't emit instructions for elems as these were not popped from
                        // the stack by the final member op during the lookup of a null
                        // coalesce assignment.
                        if null_coalesce_assignment {
                            instr::empty(alloc)
                        } else {
                            InstrSeq::gather(
                                alloc,
                                vec![base_expr_instrs_begin, elem_instrs, base_expr_instrs_end],
                            )
                        },
                        rhs_instrs,
                        InstrSeq::gather(
                            alloc,
                            vec![
                                emit_pos(alloc, pos),
                                warninstr,
                                base_setup_instrs,
                                final_instr,
                            ],
                        ),
                    )
                }
            },
            E_::ObjGet(x) if x.as_ref().3 == ast::PropOrMethod::IsProp => {
                let (e1, e2, nullflavor, _) = &**x;
                if nullflavor.eq(&ast_defs::OgNullFlavor::OGNullsafe) {
                    return Err(emit_fatal::raise_fatal_parse(
                        pos,
                        "?-> is not allowed in write context",
                    ));
                }
                let mode = match op {
                    LValOp::Unset => MemberOpMode::Unset,
                    _ => MemberOpMode::Define,
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
                let final_instr = emit_pos_then(
                    alloc,
                    pos,
                    emit_final_member_op(alloc, total_stack_size as usize, op, mk),
                );
                (
                    // Don't emit instructions for props as these were not popped from
                    // the stack by the final member op during the lookup of a null
                    // coalesce assignment.
                    if null_coalesce_assignment {
                        instr::empty(alloc)
                    } else {
                        InstrSeq::gather(
                            alloc,
                            vec![base_expr_instrs_begin, prop_instrs, base_expr_instrs_end],
                        )
                    },
                    rhs_instrs,
                    InstrSeq::gather(alloc, vec![base_setup_instrs, final_instr]),
                )
            }
            E_::ClassGet(x) if x.as_ref().2 == ast::PropOrMethod::IsProp => {
                let (cid, prop, _) = &**x;
                let cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
                let final_instr_ = emit_final_static_op(alloc, cid, prop, op)?;
                let final_instr = emit_pos_then(alloc, pos, final_instr_);
                (
                    InstrSeq::from((alloc, emit_class_expr(e, env, cexpr, prop)?)),
                    rhs_instrs,
                    final_instr,
                )
            }
            E_::Unop(uop) => (
                instr::empty(alloc),
                rhs_instrs,
                InstrSeq::gather(
                    alloc,
                    vec![
                        emit_lval_op_nonlist(
                            e,
                            env,
                            pos,
                            op,
                            &uop.1,
                            instr::empty(alloc),
                            rhs_stack_size,
                            false,
                            false, // all unary operations (++, --, etc) are on primitives, so no HHVM readonly checks
                        )?,
                        from_unop(alloc, e.options(), &uop.0)?,
                    ],
                ),
            ),
            _ => {
                return Err(emit_fatal::raise_fatal_parse(
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
    cexpr: ClassExpr<'arena>,
    prop: &ast::ClassGetExpr,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    let load_prop = |e: &mut Emitter<'arena, 'decl>| match prop {
        ast::ClassGetExpr::CGstring((pos, id)) => Ok(emit_pos_then(
            e.alloc,
            pos,
            instr::string(e.alloc, string_utils::locals::strip_dollar(id)),
        )),
        ast::ClassGetExpr::CGexpr(expr) => emit_expr(e, env, expr),
    };
    let alloc = env.arena;
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
                instr::empty(alloc),
                InstrSeq::gather(
                    alloc,
                    vec![
                        cexpr_local,
                        scope::stash_top_in_unnamed_local(e, load_prop)?,
                        instr::classgetc(alloc),
                    ],
                ),
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

pub fn fixup_type_arg<'a, 'b, 'arena>(
    env: &Env<'b, 'arena>,
    isas: bool,
    hint: &'a ast::Hint,
) -> Result<impl AsRef<ast::Hint> + 'a> {
    struct Checker<'s> {
        erased_tparams: &'s [&'s str],
        isas: bool,
    }
    impl<'ast, 's> Visitor<'ast> for Checker<'s> {
        type P = AstParams<(), Option<Error>>;

        fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
            self
        }

        fn visit_hint_fun(
            &mut self,
            c: &mut (),
            hf: &ast::HintFun,
        ) -> StdResult<(), Option<Error>> {
            hf.param_tys.accept(c, self.object())?;
            hf.return_ty.accept(c, self.object())
        }

        fn visit_hint(&mut self, c: &mut (), h: &ast::Hint) -> StdResult<(), Option<Error>> {
            use ast::{Hint_ as H_, Id};
            match h.1.as_ref() {
                H_::Happly(Id(_, id), _)
                    if self.erased_tparams.contains(&id.as_str()) && self.isas =>
                {
                    return Err(Some(emit_fatal::raise_fatal_parse(
                        &h.0,
                        "Erased generics are not allowed in is/as expressions",
                    )));
                }
                H_::Haccess(_, _) => return Ok(()),
                _ => {}
            }
            h.recurse(c, self.object())
        }

        fn visit_hint_(&mut self, c: &mut (), h: &ast::Hint_) -> StdResult<(), Option<Error>> {
            use ast::{Hint_ as H_, Id};
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
        type P = AstParams<(), ()>;

        fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
            self
        }

        fn visit_hint_fun(&mut self, c: &mut (), hf: &mut ast::HintFun) -> StdResult<(), ()> {
            <Vec<ast::Hint> as NodeMut<Self::P>>::accept(&mut hf.param_tys, c, self.object())?;
            <ast::Hint as NodeMut<Self::P>>::accept(&mut hf.return_ty, c, self.object())
        }

        fn visit_hint_(&mut self, c: &mut (), h: &mut ast::Hint_) -> StdResult<(), ()> {
            use ast::{Hint_ as H_, Id};
            match h {
                H_::Happly(Id(_, id), _) if self.erased_tparams.contains(&id.as_str()) => {
                    Ok(*id = "_".into())
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
        Ok(()) => Ok(Either::Left(hint)),
        Err(Some(error)) => Err(error),
        Err(None) => {
            let mut updater = Updater { erased_tparams };
            let mut hint = hint.clone();
            visit_mut(&mut updater, &mut (), &mut hint).unwrap();
            Ok(Either::Right(hint))
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
        type P = AstParams<(), ()>;

        fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
            self
        }

        fn visit_hint_(&mut self, c: &mut (), h_: &'ast ast::Hint_) -> StdResult<(), ()> {
            use ast::{Hint_ as H_, Id};
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
    let hint = hint.as_ref();
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
        .fold(HashSet::<&str>::default(), |acc, t| f(acc, &*t));
    let class_tparams = env.scope.get_class_tparams();
    let current_tags = class_tparams
        .iter()
        .fold(current_tags, |acc, t| f(acc, &*t));
    let mut collector = Collector {
        current_tags: &current_tags,
        acc: IndexSet::new(),
    };
    visit(&mut collector, &mut (), hint).unwrap();
    match hint.1.as_ref() {
        ast::Hint_::Happly(ast::Id(_, name), hs)
            if hs.is_empty() && current_tags.contains(name.as_str()) =>
        {
            Ok((emit_reified_type(env, pos, name)?, false))
        }
        _ => {
            let alloc = env.arena;
            let ts = get_type_structure_for_hint(e, &[], &collector.acc, hint)?;
            let ts_list = if collector.acc.is_empty() {
                ts
            } else {
                let values = collector
                    .acc
                    .iter()
                    .map(|v| emit_reified_type(env, pos, v))
                    .collect::<Result<Vec<_>>>()?;
                InstrSeq::gather(alloc, vec![InstrSeq::gather(alloc, values), ts])
            };
            Ok((
                InstrSeq::gather(
                    alloc,
                    vec![
                        ts_list,
                        instr::combine_and_resolve_type_struct(
                            alloc,
                            (collector.acc.len() + 1) as isize,
                        ),
                    ],
                ),
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
) -> std::result::Result<Local<'arena>, hhbc_by_ref_instruction_sequence::Error> {
    let alloc: &'arena bumpalo::Bump = env.arena;
    if s == special_idents::DOLLAR_DOLLAR {
        match &env.pipe_var {
            None => Err(emit_fatal::raise_fatal_runtime(
                pos,
                "Pipe variables must occur only in the RHS of pipe expressions",
            )),
            Some(var) => Ok(*var),
        }
    } else if special_idents::is_tmp_var(s) {
        Ok(*e.local_gen().get_unnamed_for_tempname(s))
    } else {
        Ok(Local::Named(Str::new_str(alloc, s)))
    }
}

pub fn emit_is_null<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    if let Some(ast::Lid(pos, id)) = expr.2.as_lvar() {
        if !is_local_this(env, id) {
            return Ok(instr::istypel(
                alloc,
                get_local(e, env, pos, local_id::get_name(id))?,
                IstypeOp::OpNull,
            ));
        }
    }

    Ok(InstrSeq::gather(
        alloc,
        vec![
            emit_expr(e, env, expr)?,
            instr::istypec(alloc, IstypeOp::OpNull),
        ],
    ))
}

pub fn emit_jmpnz<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    expr: &ast::Expr,
    label: Label,
) -> Result<EmitJmpResult<'arena>> {
    let alloc: &'arena bumpalo::Bump = env.arena;
    let ast::Expr(_, pos, expr_) = expr;
    let opt = optimize_null_checks(e);
    Ok(
        match ast_constant_folder::expr_to_typed_value(alloc, e, expr) {
            Ok(tv) => {
                if Into::<bool>::into(tv) {
                    EmitJmpResult {
                        instrs: emit_pos_then(alloc, pos, instr::jmp(alloc, label)),
                        is_fallthrough: false,
                        is_label_used: true,
                    }
                } else {
                    EmitJmpResult {
                        instrs: emit_pos_then(alloc, pos, instr::empty(alloc)),
                        is_fallthrough: true,
                        is_label_used: false,
                    }
                }
            }
            Err(_) => {
                use {ast::Expr_ as E, ast_defs::Uop as U};
                match expr_ {
                    E::Unop(uo) if uo.0 == U::Unot => emit_jmpz(e, env, &uo.1, label)?,
                    E::Binop(bo) if bo.0.is_barbar() => {
                        let r1 = emit_jmpnz(e, env, &bo.1, label)?;
                        if r1.is_fallthrough {
                            let r2 = emit_jmpnz(e, env, &bo.2, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    alloc,
                                    pos,
                                    InstrSeq::gather(alloc, vec![r1.instrs, r2.instrs]),
                                ),
                                is_fallthrough: r2.is_fallthrough,
                                is_label_used: r1.is_label_used || r2.is_label_used,
                            }
                        } else {
                            r1
                        }
                    }
                    E::Binop(bo) if bo.0.is_ampamp() => {
                        let skip_label = e.label_gen_mut().next_regular();
                        let r1 = emit_jmpz(e, env, &bo.1, skip_label)?;
                        if !r1.is_fallthrough {
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    alloc,
                                    pos,
                                    InstrSeq::gather(
                                        alloc,
                                        if r1.is_label_used {
                                            vec![r1.instrs, instr::label(alloc, skip_label)]
                                        } else {
                                            vec![r1.instrs]
                                        },
                                    ),
                                ),
                                is_fallthrough: r1.is_label_used,
                                is_label_used: false,
                            }
                        } else {
                            let r2 = emit_jmpnz(e, env, &bo.2, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    alloc,
                                    pos,
                                    InstrSeq::gather(
                                        alloc,
                                        if r1.is_label_used {
                                            vec![
                                                r1.instrs,
                                                r2.instrs,
                                                instr::label(alloc, skip_label),
                                            ]
                                        } else {
                                            vec![r1.instrs, r2.instrs]
                                        },
                                    ),
                                ),
                                is_fallthrough: r2.is_fallthrough || r1.is_label_used,
                                is_label_used: r2.is_label_used,
                            }
                        }
                    }
                    E::Binop(bo)
                        if bo.0.is_eqeqeq()
                            && ((bo.1).2.is_null() || (bo.2).2.is_null())
                            && opt =>
                    {
                        let is_null =
                            emit_is_null(e, env, if (bo.1).2.is_null() { &bo.2 } else { &bo.1 })?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                alloc,
                                pos,
                                InstrSeq::gather(alloc, vec![is_null, instr::jmpnz(alloc, label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    E::Binop(bo)
                        if bo.0.is_diff2() && ((bo.1).2.is_null() || (bo.2).2.is_null()) && opt =>
                    {
                        let is_null =
                            emit_is_null(e, env, if (bo.1).2.is_null() { &bo.2 } else { &bo.1 })?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                alloc,
                                pos,
                                InstrSeq::gather(alloc, vec![is_null, instr::jmpz(alloc, label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    _ => {
                        let instr = emit_expr(e, env, expr)?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                alloc,
                                pos,
                                InstrSeq::gather(alloc, vec![instr, instr::jmpnz(alloc, label)]),
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
) -> std::result::Result<EmitJmpResult<'arena>, hhbc_by_ref_instruction_sequence::Error> {
    let alloc: &'arena bumpalo::Bump = env.arena;
    let ast::Expr(_, pos, expr_) = expr;
    let opt = optimize_null_checks(e);
    Ok(
        match ast_constant_folder::expr_to_typed_value(alloc, e, expr) {
            Ok(v) => {
                let b: bool = v.into();
                if b {
                    EmitJmpResult {
                        instrs: emit_pos_then(alloc, pos, instr::empty(alloc)),
                        is_fallthrough: true,
                        is_label_used: false,
                    }
                } else {
                    EmitJmpResult {
                        instrs: emit_pos_then(alloc, pos, instr::jmp(alloc, label)),
                        is_fallthrough: false,
                        is_label_used: true,
                    }
                }
            }
            Err(_) => {
                use {ast::Expr_ as E, ast_defs::Uop as U};
                match expr_ {
                    E::Unop(uo) if uo.0 == U::Unot => emit_jmpnz(e, env, &uo.1, label)?,
                    E::Binop(bo) if bo.0.is_barbar() => {
                        let skip_label = e.label_gen_mut().next_regular();
                        let r1 = emit_jmpnz(e, env, &bo.1, skip_label)?;
                        if !r1.is_fallthrough {
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    alloc,
                                    pos,
                                    InstrSeq::gather(
                                        alloc,
                                        if r1.is_label_used {
                                            vec![r1.instrs, instr::label(alloc, skip_label)]
                                        } else {
                                            vec![r1.instrs]
                                        },
                                    ),
                                ),
                                is_fallthrough: r1.is_label_used,
                                is_label_used: false,
                            }
                        } else {
                            let r2 = emit_jmpz(e, env, &bo.2, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    alloc,
                                    pos,
                                    InstrSeq::gather(
                                        alloc,
                                        if r1.is_label_used {
                                            vec![
                                                r1.instrs,
                                                r2.instrs,
                                                instr::label(alloc, skip_label),
                                            ]
                                        } else {
                                            vec![r1.instrs, r2.instrs]
                                        },
                                    ),
                                ),
                                is_fallthrough: r2.is_fallthrough || r1.is_label_used,
                                is_label_used: r2.is_label_used,
                            }
                        }
                    }
                    E::Binop(bo) if bo.0.is_ampamp() => {
                        let r1 = emit_jmpz(e, env, &bo.1, label)?;
                        if r1.is_fallthrough {
                            let r2 = emit_jmpz(e, env, &bo.2, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    alloc,
                                    pos,
                                    InstrSeq::gather(alloc, vec![r1.instrs, r2.instrs]),
                                ),
                                is_fallthrough: r2.is_fallthrough,
                                is_label_used: r1.is_label_used || r2.is_label_used,
                            }
                        } else {
                            EmitJmpResult {
                                instrs: emit_pos_then(alloc, pos, r1.instrs),
                                is_fallthrough: false,
                                is_label_used: r1.is_label_used,
                            }
                        }
                    }
                    E::Binop(bo)
                        if bo.0.is_eqeqeq()
                            && ((bo.1).2.is_null() || (bo.2).2.is_null())
                            && opt =>
                    {
                        let is_null =
                            emit_is_null(e, env, if (bo.1).2.is_null() { &bo.2 } else { &bo.1 })?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                alloc,
                                pos,
                                InstrSeq::gather(alloc, vec![is_null, instr::jmpz(alloc, label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    E::Binop(bo)
                        if bo.0.is_diff2() && ((bo.1).2.is_null() || (bo.2).2.is_null()) && opt =>
                    {
                        let is_null =
                            emit_is_null(e, env, if (bo.1).2.is_null() { &bo.2 } else { &bo.1 })?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                alloc,
                                pos,
                                InstrSeq::gather(alloc, vec![is_null, instr::jmpnz(alloc, label)]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    _ => {
                        let instr = emit_expr(e, env, expr)?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                alloc,
                                pos,
                                InstrSeq::gather(alloc, vec![instr, instr::jmpz(alloc, label)]),
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

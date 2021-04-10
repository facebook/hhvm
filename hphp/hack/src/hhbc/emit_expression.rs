// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_class_expr_rust::ClassExpr;
use ast_constant_folder_rust as ast_constant_folder;
use emit_adata_rust as emit_adata;
use emit_fatal_rust as emit_fatal;
use emit_pos_rust::{emit_pos, emit_pos_then};
use emit_symbol_refs_rust as emit_symbol_refs;
use emit_type_constant_rust as emit_type_constant;
use env::{emitter::Emitter, Env, Flags as EnvFlags};
use hhas_symbol_refs_rust::IncludePath;
use hhbc_ast_rust::*;
use hhbc_id_rust::{class, r#const, function, method, prop, Id};
use hhbc_string_utils_rust as string_utils;
use instruction_sequence::{
    instr, unrecoverable,
    Error::{self, Unrecoverable},
    InstrSeq, Result,
};
use itertools::{Either, Itertools};
use label_rust::Label;
use lazy_static::lazy_static;
use naming_special_names_rust::{
    emitter_special_functions, fb, pseudo_consts, pseudo_functions, special_functions,
    special_idents, superglobals, typehints, user_attributes,
};
use options::{CompilerFlags, HhvmFlags, LangFlags, Options};
use oxidized::{
    aast, aast_defs,
    aast_visitor::{visit, visit_mut, AstParams, Node, NodeMut, Visitor, VisitorMut},
    ast as tast, ast_defs, local_id,
    pos::Pos,
};
use regex::Regex;
use runtime::TypedValue;
use scope_rust::scope;

use indexmap::IndexSet;
use std::{
    collections::{BTreeMap, HashSet},
    convert::TryInto,
    iter,
    result::Result as StdResult,
    str::FromStr,
};

#[derive(Debug)]
pub struct EmitJmpResult {
    // generated instruction sequence
    pub instrs: InstrSeq,
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

pub fn is_local_this(env: &Env, lid: &local_id::LocalId) -> bool {
    local_id::get_name(lid) == special_idents::THIS
        && env.scope.has_this()
        && !env.scope.is_toplevel()
}

mod inout_locals {
    use crate::*;
    use oxidized::{aast_defs::Lid, aast_visitor, aast_visitor::Node, ast as tast, ast_defs};
    use std::{collections::HashMap, marker::PhantomData};

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

    pub(super) type AliasInfoMap = HashMap<String, AliasInfo>;

    fn add_write(name: String, i: usize, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_write(i as isize);
    }

    fn add_inout(name: String, i: usize, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_inout(i as isize);
    }

    fn add_use(name: String, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_use();
    }

    // determines if value of a local 'name' that appear in parameter 'i'
    // should be saved to local because it might be overwritten later
    pub(super) fn should_save_local_value(name: &str, i: usize, aliases: &AliasInfoMap) -> bool {
        aliases
            .get(name)
            .map_or(false, |alias| alias.in_range(i as isize))
    }

    pub(super) fn should_move_local_value(local: &local::Type, aliases: &AliasInfoMap) -> bool {
        match local {
            local::Type::Named(name) => aliases
                .get(&**name)
                .map_or(true, |alias| alias.has_single_ref()),
            local::Type::Unnamed(_) => false,
        }
    }

    pub(super) fn collect_written_variables(env: &Env, args: &[tast::Expr]) -> AliasInfoMap {
        let mut acc = HashMap::new();
        args.iter()
            .enumerate()
            .for_each(|(i, arg)| handle_arg(env, true, i, arg, &mut acc));
        acc
    }

    fn handle_arg(env: &Env, is_top: bool, i: usize, arg: &tast::Expr, acc: &mut AliasInfoMap) {
        use tast::{Expr, Expr_};
        let Expr(_, e) = arg;
        // inout $v
        if let Some((ast_defs::ParamKind::Pinout, Expr(_, Expr_::Lvar(lid)))) = e.as_callconv() {
            let Lid(_, lid) = &**lid;
            if !is_local_this(env, &lid) {
                add_use(lid.1.to_string(), acc);
                return if is_top {
                    add_inout(lid.1.to_string(), i, acc);
                } else {
                    add_write(lid.1.to_string(), i, acc);
                };
            }
        }
        // $v
        if let Some(Lid(_, (_, id))) = e.as_lvar() {
            return add_use(id.to_string(), acc);
        }
        // dive into argument value
        aast_visitor::visit(
            &mut Visitor {
                phantom: PhantomData,
            },
            &mut Ctx { state: acc, env, i },
            arg,
        )
        .unwrap();
    }

    struct Visitor<'a> {
        phantom: PhantomData<&'a str>,
    }

    pub struct Ctx<'a> {
        // TODO(shiqicao): Change AliasInfoMap to AliasInfoMap<'ast>
        state: &'a mut AliasInfoMap,
        env: &'a Env<'a>,
        i: usize,
    }

    impl<'ast, 'a> aast_visitor::Visitor<'ast> for Visitor<'a> {
        type P = aast_visitor::AstParams<Ctx<'a>, ()>;

        fn object(&mut self) -> &mut dyn aast_visitor::Visitor<'ast, P = Self::P> {
            self
        }

        fn visit_expr_(
            &mut self,
            c: &mut Ctx<'a>,
            p: &'ast tast::Expr_,
        ) -> std::result::Result<(), ()> {
            // f(inout $v) or f(&$v)
            if let tast::Expr_::Call(expr) = p {
                let (_, _, args, uarg) = &**expr;
                args.iter()
                    .for_each(|arg| handle_arg(&c.env, false, c.i, arg, &mut c.state));
                uarg.as_ref()
                    .map(|arg| handle_arg(&c.env, false, c.i, arg, &mut c.state));
                Ok(())
            } else {
                p.recurse(c, self.object())?;
                Ok(match p {
                    // lhs op= _
                    tast::Expr_::Binop(expr) => {
                        let (bop, left, _) = &**expr;
                        if let ast_defs::Bop::Eq(_) = bop {
                            collect_lvars_hs(c, left)
                        }
                    }
                    // $i++ or $i--
                    tast::Expr_::Unop(expr) => {
                        let (uop, e) = &**expr;
                        match uop {
                            ast_defs::Uop::Uincr | ast_defs::Uop::Udecr => collect_lvars_hs(c, e),
                            _ => {}
                        }
                    }
                    // $v
                    tast::Expr_::Lvar(expr) => {
                        let Lid(_, (_, id)) = &**expr;
                        add_use(id.to_string(), &mut c.state);
                    }
                    _ => {}
                })
            }
        }
    }

    // collect lvars on the left hand side of '=' operator
    fn collect_lvars_hs(ctx: &mut Ctx, expr: &tast::Expr) {
        let tast::Expr(_, e) = expr;
        match &*e {
            tast::Expr_::Lvar(lid) => {
                let Lid(_, lid) = &**lid;
                if !is_local_this(&ctx.env, &lid) {
                    add_use(lid.1.to_string(), &mut ctx.state);
                    add_write(lid.1.to_string(), ctx.i, &mut ctx.state);
                }
            }
            tast::Expr_::List(exprs) => exprs.iter().for_each(|expr| collect_lvars_hs(ctx, expr)),
            _ => {}
        }
    }
}

pub fn get_type_structure_for_hint(
    e: &mut Emitter,
    tparams: &[&str],
    targ_map: &IndexSet<&str>,
    hint: &aast::Hint,
) -> Result<InstrSeq> {
    let targ_map: BTreeMap<&str, i64> = targ_map
        .iter()
        .enumerate()
        .map(|(i, n)| (*n, i as i64))
        .collect();
    let tv = emit_type_constant::hint_to_type_constant(
        e.options(),
        tparams,
        &targ_map,
        &hint,
        false,
        false,
    )?;
    let i = emit_adata::get_array_identifier(e, &tv);
    Ok(instr::lit_const(InstructLitConst::Dict(i)))
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
type InstrSeqWithLocals = Vec<(InstrSeq, Option<(local::Type, StoredValueKind)>)>;

/// result of emit_array_get
enum ArrayGetInstr {
    /// regular $a[..] that does not need to spill anything
    Regular(InstrSeq),
    /// subscript expression used as inout argument that need to spill intermediate values:
    Inout {
        /// instruction sequence with locals to load value
        load: InstrSeqWithLocals,
        /// instruction to set value back (can use locals defined in load part)
        store: InstrSeq,
    },
}

struct ArrayGetBaseData<T> {
    base_instrs: T,
    cls_instrs: InstrSeq,
    setup_instrs: InstrSeq,
    base_stack_size: StackIndex,
    cls_stack_size: StackIndex,
}

/// result of emit_base
enum ArrayGetBase {
    /// regular <base> part in <base>[..] that does not need to spill anything
    Regular(ArrayGetBaseData<InstrSeq>),
    /// base of subscript expression used as inout argument that need to spill
    /// intermediate values
    Inout {
        /// instructions to load base part
        load: ArrayGetBaseData<InstrSeqWithLocals>,
        /// instruction to load base part for setting inout argument back
        store: InstrSeq,
    },
}

pub fn emit_expr(emitter: &mut Emitter, env: &Env, expression: &tast::Expr) -> Result {
    use aast_defs::Lid;
    use tast::Expr_;
    let tast::Expr(pos, expr) = expression;
    match expr {
        Expr_::Float(_)
        | Expr_::EnumAtom(_)
        | Expr_::String(_)
        | Expr_::Int(_)
        | Expr_::Null
        | Expr_::False
        | Expr_::True => {
            let v = ast_constant_folder::expr_to_typed_value(emitter, &env.namespace, expression)
                .map_err(|_| unrecoverable("expr_to_typed_value failed"))?;
            Ok(emit_pos_then(pos, instr::typedvalue(v)))
        }
        Expr_::PrefixedString(e) => emit_expr(emitter, env, &e.1),
        Expr_::Lvar(e) => {
            let Lid(pos, _) = &**e;
            Ok(InstrSeq::gather(vec![
                emit_pos(pos),
                emit_local(emitter, env, BareThisOp::Notice, e)?,
            ]))
        }
        Expr_::ClassConst(e) => emit_class_const(emitter, env, pos, &e.0, &e.1),
        Expr_::Unop(e) => emit_unop(emitter, env, pos, e),
        Expr_::Binop(_) => emit_binop(emitter, env, pos, expression),
        Expr_::Pipe(e) => emit_pipe(emitter, env, e),
        Expr_::Is(is_expr) => {
            let (e, h) = &**is_expr;
            let is = emit_is(emitter, env, pos, h)?;
            Ok(InstrSeq::gather(vec![emit_expr(emitter, env, e)?, is]))
        }
        Expr_::As(e) => emit_as(emitter, env, pos, e),
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
        Expr_::ObjGet(e) => {
            if e.3 {
                // Case ($x->foo).
                let e = tast::Expr(
                    pos.clone(),
                    Expr_::ObjGet(Box::new((e.0.clone(), e.1.clone(), e.2.clone(), false))),
                );
                emit_expr(emitter, env, &e)
            } else {
                // Case $x->foo.
                Ok(emit_obj_get(emitter, env, pos, QueryOp::CGet, &e.0, &e.1, &e.2, e.3)?.0)
            }
        }
        Expr_::Call(c) => emit_call_expr(emitter, env, pos, None, c),
        Expr_::New(e) => emit_new(emitter, env, pos, e, false),
        Expr_::FunctionPointer(fp) => emit_function_pointer(emitter, env, pos, &fp.0, &fp.1),
        Expr_::Record(e) => emit_record(emitter, env, pos, e),
        Expr_::Darray(e) => Ok(emit_pos_then(
            pos,
            emit_collection(emitter, env, expression, &mk_afkvalues(&e.1), None)?,
        )),
        Expr_::Varray(e) => Ok(emit_pos_then(
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
            let fields = mk_afvalues(&vec![e1, e2]);
            emit_named_collection(emitter, env, pos, expression, &fields, CollectionType::Pair)
        }
        Expr_::KeyValCollection(e) => {
            let (kind, _, fields) = &**e;
            let fields = mk_afkvalues(
                &fields
                    .to_owned()
                    .into_iter()
                    .map(|tast::Field(e1, e2)| (e1, e2))
                    .collect(),
            );
            let collection_typ = match kind {
                aast_defs::KvcKind::Map => CollectionType::Map,
                aast_defs::KvcKind::ImmMap => CollectionType::ImmMap,
                _ => return emit_collection(emitter, env, expression, &fields, None),
            };
            emit_named_collection(emitter, env, pos, expression, &fields, collection_typ)
        }
        Expr_::Clone(e) => Ok(emit_pos_then(pos, emit_clone(emitter, env, e)?)),
        Expr_::Shape(e) => Ok(emit_pos_then(pos, emit_shape(emitter, env, expression, e)?)),
        Expr_::Await(e) => emit_await(emitter, env, pos, e),
        // TODO: emit readonly expressions
        Expr_::ReadonlyExpr(e) => emit_expr(emitter, env, e),
        Expr_::Yield(e) => emit_yield(emitter, env, pos, e),
        Expr_::Efun(e) => Ok(emit_pos_then(pos, emit_lambda(emitter, env, &e.0, &e.1)?)),

        Expr_::ClassGet(e) => {
            if !e.2 {
                emit_class_get(emitter, env, QueryOp::CGet, &e.0, &e.1)
            } else {
                let e = tast::Expr(
                    pos.clone(),
                    Expr_::ClassGet(Box::new((e.0.clone(), e.1.clone(), false))),
                );
                emit_expr(emitter, env, &e)
            }
        }

        Expr_::String2(es) => emit_string2(emitter, env, pos, es),
        Expr_::Id(e) => Ok(emit_pos_then(pos, emit_id(emitter, env, e)?)),
        Expr_::Xml(_) => Err(unrecoverable(
            "emit_xhp: syntax should have been converted during rewriting",
        )),
        Expr_::Callconv(_) => Err(unrecoverable(
            "emit_callconv: This should have been caught at emit_arg",
        )),
        Expr_::Import(e) => emit_import(emitter, env, pos, &e.0, &e.1),
        Expr_::Omitted => Ok(instr::empty()),
        Expr_::Lfun(_) => Err(unrecoverable(
            "expected Lfun to be converted to Efun during closure conversion emit_expr",
        )),
        Expr_::List(_) => Err(emit_fatal::raise_fatal_parse(
            pos,
            "list() can only be used as an lvar. Did you mean to use tuple()?",
        )),
        Expr_::Tuple(_) => {
            unimplemented!("TODO: generate Tuple nodes in lowerer and update codegen")
        }

        Expr_::Any => Err(unrecoverable("Cannot codegen from an Any node")),
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

fn emit_exprs(e: &mut Emitter, env: &Env, exprs: &[tast::Expr]) -> Result {
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

fn emit_id(emitter: &mut Emitter, env: &Env, id: &tast::Sid) -> Result {
    use pseudo_consts::*;
    use InstructLitConst::*;

    let ast_defs::Id(p, s) = id;
    let res = match s.as_str() {
        G__FILE__ => instr::lit_const(File),
        G__DIR__ => instr::lit_const(Dir),
        G__METHOD__ => instr::lit_const(Method),
        G__FUNCTION_CREDENTIAL__ => instr::lit_const(FuncCred),
        G__CLASS__ => InstrSeq::gather(vec![instr::self_(), instr::classname()]),
        G__COMPILER_FRONTEND__ => instr::string("hackc"),
        G__LINE__ => instr::int(p.info_pos_extended().1.try_into().map_err(|_| {
            emit_fatal::raise_fatal_parse(p, "error converting end of line from usize to isize")
        })?),
        G__NAMESPACE__ => instr::string(env.namespace.name.as_ref().map_or("", |s| &s[..])),
        EXIT | DIE => return emit_exit(emitter, env, None),
        _ => {
            // panic!("TODO: uncomment after D19350786 lands")
            // let cid: ConstId = r#const::Type::from_ast_name(&s);
            let cid: ConstId = string_utils::strip_global_ns(&s).to_string().into();
            emit_symbol_refs::add_constant(emitter, cid.clone());
            return Ok(emit_pos_then(p, instr::lit_const(CnsE(cid))));
        }
    };
    Ok(res)
}

fn emit_exit(emitter: &mut Emitter, env: &Env, expr_opt: Option<&tast::Expr>) -> Result {
    Ok(InstrSeq::gather(vec![
        expr_opt.map_or_else(|| Ok(instr::int(0)), |e| emit_expr(emitter, env, e))?,
        instr::exit(),
    ]))
}

fn emit_yield(e: &mut Emitter, env: &Env, pos: &Pos, af: &tast::Afield) -> Result {
    Ok(match af {
        tast::Afield::AFvalue(v) => {
            InstrSeq::gather(vec![emit_expr(e, env, v)?, emit_pos(pos), instr::yield_()])
        }
        tast::Afield::AFkvalue(k, v) => InstrSeq::gather(vec![
            emit_expr(e, env, k)?,
            emit_expr(e, env, v)?,
            emit_pos(pos),
            instr::yieldk(),
        ]),
    })
}

fn parse_include(e: &tast::Expr) -> IncludePath {
    fn strip_backslash(s: &mut String) {
        if s.starts_with("/") {
            *s = s[1..].into()
        }
    }
    fn split_var_lit(e: &tast::Expr) -> (String, String) {
        match &e.1 {
            tast::Expr_::Binop(x) if x.0.is_dot() => {
                let (v, l) = split_var_lit(&x.2);
                if v.is_empty() {
                    let (var, lit) = split_var_lit(&x.1);
                    (var, format!("{}{}", lit, l))
                } else {
                    (v, String::new())
                }
            }
            tast::Expr_::String(lit) => (String::new(), lit.to_string()),
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
            IncludePath::SearchPathRelative(lit)
        } else {
            IncludePath::Absolute(lit)
        }
    } else {
        strip_backslash(&mut lit);
        IncludePath::IncludeRootRelative(var, lit)
    }
}

fn text_of_expr(e: &tast::Expr) -> String {
    match &e.1 {
        tast::Expr_::String(s) => format!("\'{}\'", s),
        tast::Expr_::Id(id) => id.1.to_string(),
        tast::Expr_::Lvar(lid) => local_id::get_name(&lid.1).to_string(),
        tast::Expr_::ArrayGet(x) => match ((x.0).1.as_lvar(), x.1.as_ref()) {
            (Some(tast::Lid(_, id)), Some(e_)) => {
                format!("{}[{}]", local_id::get_name(&id), text_of_expr(e_))
            }
            _ => "unknown".into(),
        },
        _ => "unknown".into(),
    }
}

fn text_of_class_id(cid: &tast::ClassId) -> String {
    match &cid.1 {
        tast::ClassId_::CIparent => "parent".into(),
        tast::ClassId_::CIself => "self".into(),
        tast::ClassId_::CIstatic => "static".into(),
        tast::ClassId_::CIexpr(e) => text_of_expr(e),
        tast::ClassId_::CI(ast_defs::Id(_, id)) => id.into(),
    }
}

fn text_of_prop(prop: &tast::ClassGetExpr) -> String {
    match prop {
        tast::ClassGetExpr::CGstring((_, s)) => s.into(),
        tast::ClassGetExpr::CGexpr(e) => text_of_expr(e),
    }
}

fn emit_import(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    flavor: &tast::ImportFlavor,
    expr: &tast::Expr,
) -> Result {
    use tast::ImportFlavor;
    let inc = parse_include(expr);
    emit_symbol_refs::add_include(e, inc.clone());
    let (expr_instrs, import_op_instr) = match flavor {
        ImportFlavor::Include => (emit_expr(e, env, expr)?, instr::incl()),
        ImportFlavor::Require => (emit_expr(e, env, expr)?, instr::req()),
        ImportFlavor::IncludeOnce => (emit_expr(e, env, expr)?, instr::inclonce()),
        ImportFlavor::RequireOnce => {
            match inc.into_doc_root_relative(e.options().hhvm.include_roots.get()) {
                IncludePath::DocRootRelative(path) => {
                    let expr = tast::Expr(pos.clone(), tast::Expr_::String(path.into()));
                    (emit_expr(e, env, &expr)?, instr::reqdoc())
                }
                _ => (emit_expr(e, env, expr)?, instr::reqonce()),
            }
        }
    };
    Ok(InstrSeq::gather(vec![
        expr_instrs,
        emit_pos(pos),
        import_op_instr,
    ]))
}

fn emit_string2(e: &mut Emitter, env: &Env, pos: &Pos, es: &Vec<tast::Expr>) -> Result {
    if es.is_empty() {
        Err(unrecoverable("String2 with zero araguments is impossible"))
    } else if es.len() == 1 {
        Ok(InstrSeq::gather(vec![
            emit_expr(e, env, &es[0])?,
            emit_pos(pos),
            instr::cast_string(),
        ]))
    } else {
        Ok(InstrSeq::gather(vec![
            emit_two_exprs(e, env, &es[0].0, &es[0], &es[1])?,
            emit_pos(pos),
            instr::concat(),
            InstrSeq::gather(
                (&es[2..])
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

fn emit_clone(e: &mut Emitter, env: &Env, expr: &tast::Expr) -> Result {
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        instr::clone(),
    ]))
}

fn emit_lambda(e: &mut Emitter, env: &Env, fndef: &tast::Fun_, ids: &[aast_defs::Lid]) -> Result {
    // Closure conversion puts the class number used for CreateCl in the "name"
    // of the function definition
    let fndef_name = &(fndef.name).1;
    let cls_num = fndef_name
        .parse::<isize>()
        .map_err(|err| Unrecoverable(err.to_string()))?;
    let explicit_use = e.emit_global_state().explicit_use_set.contains(fndef_name);
    let is_in_lambda = env.scope.is_in_lambda();
    Ok(InstrSeq::gather(vec![
        InstrSeq::gather(
            ids.iter()
                .map(|tast::Lid(pos, id)| {
                    match string_utils::reified::is_captured_generic(local_id::get_name(id)) {
                        Some((is_fun, i)) => {
                            if is_in_lambda {
                                Ok(instr::cgetl(local::Type::Named(
                                    string_utils::reified::reified_generic_captured_name(
                                        is_fun, i as usize,
                                    ),
                                )))
                            } else {
                                emit_reified_generic_instrs(&Pos::make_none(), is_fun, i as usize)
                            }
                        }
                        None => Ok({
                            let lid = get_local(e, env, pos, local_id::get_name(id))?;
                            if explicit_use {
                                instr::cgetl(lid)
                            } else {
                                instr::cugetl(lid)
                            }
                        }),
                    }
                })
                .collect::<Result<Vec<_>>>()?,
        ),
        instr::createcl(ids.len(), cls_num),
    ]))
}

pub fn emit_await(emitter: &mut Emitter, env: &Env, pos: &Pos, expr: &tast::Expr) -> Result {
    let tast::Expr(_, e) = expr;
    let cant_inline_gen_functions = !emitter
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION);
    match e.as_call() {
        Some((tast::Expr(_, tast::Expr_::Id(id)), _, args, None))
            if (cant_inline_gen_functions
                && args.len() == 1
                && string_utils::strip_global_ns(&(*id.1)) == "gena") =>
        {
            return inline_gena_call(emitter, env, &args[0]);
        }
        _ => {
            let after_await = emitter.label_gen_mut().next_regular();
            let instrs = match e {
                tast::Expr_::Call(c) => {
                    emit_call_expr(emitter, env, pos, Some(after_await.clone()), &*c)?
                }
                _ => emit_expr(emitter, env, expr)?,
            };
            Ok(InstrSeq::gather(vec![
                instrs,
                emit_pos(pos),
                instr::dup(),
                instr::istypec(IstypeOp::OpNull),
                instr::jmpnz(after_await.clone()),
                instr::await_(),
                instr::label(after_await),
            ]))
        }
    }
}

fn inline_gena_call(emitter: &mut Emitter, env: &Env, arg: &tast::Expr) -> Result {
    let load_arr = emit_expr(emitter, env, arg)?;
    let async_eager_label = emitter.label_gen_mut().next_regular();

    scope::with_unnamed_local(emitter, |e, arr_local| {
        let before = InstrSeq::gather(vec![
            load_arr,
            instr::cast_dict(),
            instr::popl(arr_local.clone()),
        ]);

        let inner = InstrSeq::gather(vec![
            instr::nulluninit(),
            instr::nulluninit(),
            instr::cgetl(arr_local.clone()),
            instr::fcallclsmethodd(
                FcallArgs::new(
                    FcallFlags::default(),
                    1,
                    vec![],
                    Some(async_eager_label.clone()),
                    1,
                    None,
                ),
                method::from_raw_string("fromDict"),
                class::from_raw_string("HH\\AwaitAllWaitHandle"),
            ),
            instr::await_(),
            instr::label(async_eager_label.clone()),
            instr::popc(),
            emit_iter(
                e,
                instr::cgetl(arr_local.clone()),
                |val_local, key_local| {
                    InstrSeq::gather(vec![
                        instr::cgetl(val_local),
                        instr::whresult(),
                        instr::basel(arr_local.clone(), MemberOpMode::Define),
                        instr::setm(0, MemberKey::EL(key_local, ReadOnlyOp::Any)),
                        instr::popc(),
                    ])
                },
            )?,
        ]);

        let after = instr::pushl(arr_local);

        Ok((before, inner, after))
    })
}

fn emit_iter<F: FnOnce(local::Type, local::Type) -> InstrSeq>(
    e: &mut Emitter,
    collection: InstrSeq,
    f: F,
) -> Result {
    scope::with_unnamed_locals_and_iterators(e, |e| {
        let iter_id = e.iterator_mut().get();
        let val_id = e.local_gen_mut().get_unnamed();
        let key_id = e.local_gen_mut().get_unnamed();
        let loop_end = e.label_gen_mut().next_regular();
        let loop_next = e.label_gen_mut().next_regular();
        let iter_args = IterArgs {
            iter_id,
            key_id: Some(key_id.clone()),
            val_id: val_id.clone(),
        };
        let iter_init = InstrSeq::gather(vec![
            collection,
            instr::iterinit(iter_args.clone(), loop_end.clone()),
        ]);
        let iterate = InstrSeq::gather(vec![
            instr::label(loop_next.clone()),
            f(val_id.clone(), key_id.clone()),
            instr::iternext(iter_args, loop_next),
        ]);
        let iter_done = InstrSeq::gather(vec![
            instr::unsetl(val_id),
            instr::unsetl(key_id),
            instr::label(loop_end),
        ]);
        Ok((iter_init, iterate, iter_done))
    })
}

fn emit_shape(
    emitter: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    fl: &[(ast_defs::ShapeFieldName, tast::Expr)],
) -> Result {
    fn extract_shape_field_name_pstring(
        env: &Env,
        pos: &Pos,
        field: &ast_defs::ShapeFieldName,
    ) -> Result<tast::Expr_> {
        use ast_defs::ShapeFieldName as SF;
        Ok(match field {
            SF::SFlitInt(s) => tast::Expr_::mk_int(s.1.clone()),
            SF::SFlitStr(s) => tast::Expr_::mk_string(s.1.clone()),
            SF::SFclassConst(id, p) => {
                if is_reified_tparam(env, true, &id.1).is_some()
                    || is_reified_tparam(env, false, &id.1).is_some()
                {
                    return Err(emit_fatal::raise_fatal_parse(
                        &id.0,
                        "Reified generics cannot be used in shape keys",
                    ));
                } else {
                    tast::Expr_::mk_class_const(
                        tast::ClassId(pos.clone(), tast::ClassId_::CI(id.clone())),
                        p.clone(),
                    )
                }
            }
        })
    }
    let pos = &expr.0;
    // TODO(hrust): avoid clone
    let fl = fl
        .iter()
        .map(|(f, e)| {
            Ok((
                tast::Expr(pos.clone(), extract_shape_field_name_pstring(env, pos, f)?),
                e.clone(),
            ))
        })
        .collect::<Result<Vec<_>>>()?;
    emit_expr(
        emitter,
        env,
        &tast::Expr(pos.clone(), tast::Expr_::mk_darray(None, fl)),
    )
}

fn emit_vec_collection(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    fields: &Vec<tast::Afield>,
) -> Result {
    match ast_constant_folder::vec_to_typed_value(e, &env.namespace, fields) {
        Ok(tv) => emit_static_collection(None, pos, tv),
        Err(_) => emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec),
    }
}

fn emit_named_collection(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    expr: &tast::Expr,
    fields: &Vec<tast::Afield>,
    collection_type: CollectionType,
) -> Result {
    let emit_vector_like = |e: &mut Emitter, collection_type| {
        Ok(if fields.is_empty() {
            emit_pos_then(pos, instr::newcol(collection_type))
        } else {
            InstrSeq::gather(vec![
                emit_vec_collection(e, env, pos, fields)?,
                instr::colfromarray(collection_type),
            ])
        })
    };
    let emit_map_or_set = |e: &mut Emitter, collection_type| {
        if fields.is_empty() {
            Ok(emit_pos_then(pos, instr::newcol(collection_type)))
        } else {
            emit_collection(e, env, expr, fields, Some(collection_type))
        }
    };
    use CollectionType as C;
    match collection_type {
        C::Dict | C::Vec | C::Keyset => {
            let instr = emit_collection(e, env, expr, fields, None)?;
            Ok(emit_pos_then(pos, instr))
        }
        C::Vector | C::ImmVector => emit_vector_like(e, collection_type),
        C::Map | C::ImmMap | C::Set | C::ImmSet => emit_map_or_set(e, collection_type),
        C::Pair => Ok(InstrSeq::gather(vec![
            InstrSeq::gather(
                fields
                    .iter()
                    .map(|f| match f {
                        tast::Afield::AFvalue(v) => emit_expr(e, env, v),
                        _ => Err(unrecoverable("impossible Pair argument")),
                    })
                    .collect::<Result<_>>()?,
            ),
            instr::new_pair(),
        ])),
        _ => Err(unrecoverable("Unexpected named collection type")),
    }
}

fn emit_named_collection_str(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    (ast_defs::Id(pos, name), _, fields): &(
        tast::Sid,
        Option<tast::CollectionTarg>,
        Vec<tast::Afield>,
    ),
) -> Result {
    let name = string_utils::strip_ns(name);
    let name = string_utils::types::fix_casing(name.as_ref());
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

fn mk_afkvalues(es: &Vec<(tast::Expr, tast::Expr)>) -> Vec<tast::Afield> {
    es.to_owned()
        .into_iter()
        .map(|(e1, e2)| tast::Afield::mk_afkvalue(e1, e2))
        .collect()
}

fn mk_afvalues(es: &Vec<tast::Expr>) -> Vec<tast::Afield> {
    es.to_owned()
        .into_iter()
        .map(|e| tast::Afield::mk_afvalue(e))
        .collect()
}

fn emit_collection(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    fields: &[tast::Afield],
    transform_to_collection: Option<CollectionType>,
) -> Result {
    let pos = &expr.0;
    match ast_constant_folder::expr_to_typed_value_(
        e,
        &env.namespace,
        expr,
        true,  /*allow_map*/
        false, /*force_class_const*/
    ) {
        Ok(tv) => emit_static_collection(transform_to_collection, pos, tv),
        Err(_) => emit_dynamic_collection(e, env, expr, fields),
    }
}

fn emit_static_collection(
    transform_to_collection: Option<CollectionType>,
    pos: &Pos,
    tv: TypedValue,
) -> Result {
    let transform_instr = match transform_to_collection {
        Some(collection_type) => instr::colfromarray(collection_type),
        _ => instr::empty(),
    };
    Ok(InstrSeq::gather(vec![
        emit_pos(pos),
        instr::typedvalue(tv),
        transform_instr,
    ]))
}

fn expr_and_new(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    instr_to_add_new: InstrSeq,
    instr_to_add: InstrSeq,
    field: &tast::Afield,
) -> Result {
    match field {
        tast::Afield::AFvalue(v) => Ok(InstrSeq::gather(vec![
            emit_expr(e, env, v)?,
            emit_pos(pos),
            instr_to_add_new,
        ])),
        tast::Afield::AFkvalue(k, v) => Ok(InstrSeq::gather(vec![
            emit_two_exprs(e, env, &k.0, k, v)?,
            instr_to_add,
        ])),
    }
}

fn emit_keyvalue_collection(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    fields: &[tast::Afield],
    ctype: CollectionType,
    constructor: InstructLitConst,
) -> Result {
    let (transform_instr, add_elem_instr) = match ctype {
        CollectionType::Dict | CollectionType::Array => (instr::empty(), instr::add_new_elemc()),
        _ => (
            instr::colfromarray(ctype),
            InstrSeq::gather(vec![instr::dup(), instr::add_elemc()]),
        ),
    };
    let emitted_pos = emit_pos(pos);
    Ok(InstrSeq::gather(vec![
        emitted_pos.clone(),
        instr::lit_const(constructor),
        fields
            .iter()
            .map(|f| expr_and_new(e, env, pos, add_elem_instr.clone(), instr::add_elemc(), f))
            .collect::<Result<_>>()
            .map(InstrSeq::gather)?,
        emitted_pos,
        transform_instr,
    ]))
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
        Ok(match s.matches(".").count() {
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
            FLOAT.replace(s, "${int}.${dec}").trim_end_matches(".") > i64::MAX.to_string().as_str()
        } else if NEG_FLOAT.is_match(s) {
            NEG_FLOAT.replace(s, "${int}.${dec}").trim_end_matches(".")
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

fn is_struct_init(
    e: &mut Emitter,
    env: &Env,
    fields: &[tast::Afield],
    allow_numerics: bool,
) -> Result<bool> {
    let mut are_all_keys_non_numeric_strings = true;
    let mut uniq_keys = std::collections::HashSet::<bstr::BString>::new();
    for f in fields.iter() {
        if let tast::Afield::AFkvalue(key, _) = f {
            // TODO(hrust): if key is String, don't clone and call fold_expr
            let mut key = key.clone();
            ast_constant_folder::fold_expr(&mut key, e, &env.namespace)
                .map_err(|e| unrecoverable(format!("{}", e)))?;
            if let tast::Expr(_, tast::Expr_::String(s)) = key {
                are_all_keys_non_numeric_strings = are_all_keys_non_numeric_strings
                    && non_numeric(
                        // FIXME: This is not safe--string literals are binary strings.
                        // There's no guarantee that they're valid UTF-8.
                        unsafe { std::str::from_utf8_unchecked(s.as_slice().into()) },
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

fn emit_struct_array<C: FnOnce(&mut Emitter, Vec<String>) -> Result<InstrSeq>>(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    fields: &[tast::Afield],
    ctor: C,
) -> Result {
    use tast::{Expr as E, Expr_ as E_};
    let (keys, value_instrs) = fields
        .iter()
        .map(|f| match f {
            tast::Afield::AFkvalue(k, v) => match k {
                E(_, E_::String(s)) => Ok((
                    // FIXME: This is not safe--string literals are binary strings.
                    // There's no guarantee that they're valid UTF-8.
                    unsafe { String::from_utf8_unchecked(s.clone().into()) },
                    emit_expr(e, env, v)?,
                )),
                _ => {
                    let mut k = k.clone();
                    ast_constant_folder::fold_expr(&mut k, e, &env.namespace)
                        .map_err(|e| unrecoverable(format!("{}", e)))?;
                    match k {
                        E(_, E_::String(s)) => Ok((
                            // FIXME: This is not safe--string literals are binary strings.
                            // There's no guarantee that they're valid UTF-8.
                            unsafe { String::from_utf8_unchecked(s.clone().into()) },
                            emit_expr(e, env, v)?,
                        )),
                        _ => Err(unrecoverable("Key must be a string")),
                    }
                }
            },
            _ => Err(unrecoverable("impossible")),
        })
        .collect::<Result<Vec<(String, InstrSeq)>>>()?
        .into_iter()
        .unzip();
    Ok(InstrSeq::gather(vec![
        InstrSeq::gather(value_instrs),
        emit_pos(pos),
        ctor(e, keys)?,
    ]))
}

fn emit_dynamic_collection(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    fields: &[tast::Afield],
) -> Result {
    let pos = &expr.0;
    let count = fields.len();
    let emit_dict = |e: &mut Emitter| {
        if is_struct_init(e, env, fields, true)? {
            emit_struct_array(e, env, pos, fields, |_, x| Ok(instr::newstructdict(x)))
        } else {
            let ctor = InstructLitConst::NewDictArray(count as isize);
            emit_keyvalue_collection(e, env, pos, fields, CollectionType::Dict, ctor)
        }
    };
    let emit_collection_helper = |e: &mut Emitter, ctype| {
        if is_struct_init(e, env, fields, true)? {
            Ok(InstrSeq::gather(vec![
                emit_struct_array(e, env, pos, fields, |_, x| Ok(instr::newstructdict(x)))?,
                emit_pos(pos),
                instr::colfromarray(ctype),
            ]))
        } else {
            let ctor = InstructLitConst::NewDictArray(count as isize);
            emit_keyvalue_collection(e, env, pos, fields, ctype, ctor)
        }
    };
    use tast::Expr_ as E_;
    match &expr.1 {
        E_::ValCollection(v) if v.0 == tast::VcKind::Vec => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec)
        }
        E_::Collection(v) if (v.0).1 == "vec" => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec)
        }
        E_::ValCollection(v) if v.0 == tast::VcKind::Keyset => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewKeysetArray)
        }
        E_::Collection(v) if (v.0).1 == "keyset" => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewKeysetArray)
        }
        E_::Collection(v) if (v.0).1 == "dict" => emit_dict(e),
        E_::KeyValCollection(v) if v.0 == tast::KvcKind::Dict => emit_dict(e),
        E_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "Set" => {
            emit_collection_helper(e, CollectionType::Set)
        }
        E_::ValCollection(v) if v.0 == tast::VcKind::Set => {
            emit_collection_helper(e, CollectionType::Set)
        }
        E_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "ImmSet" => {
            emit_collection_helper(e, CollectionType::ImmSet)
        }
        E_::ValCollection(v) if v.0 == tast::VcKind::ImmSet => {
            emit_collection_helper(e, CollectionType::ImmSet)
        }
        E_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "Map" => {
            emit_collection_helper(e, CollectionType::Map)
        }
        E_::KeyValCollection(v) if v.0 == tast::KvcKind::Map => {
            emit_collection_helper(e, CollectionType::Map)
        }
        E_::Collection(v) if string_utils::strip_ns(&(v.0).1) == "ImmMap" => {
            emit_collection_helper(e, CollectionType::ImmMap)
        }
        E_::KeyValCollection(v) if v.0 == tast::KvcKind::ImmMap => {
            emit_collection_helper(e, CollectionType::ImmMap)
        }
        E_::Varray(_) => {
            let instrs = emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVec);
            Ok(instrs?)
        }
        E_::Darray(_) => {
            if is_struct_init(e, env, fields, false /* allow_numerics */)? {
                let instrs = emit_struct_array(e, env, pos, fields, |_, arg| {
                    let instr = instr::newstructdict(arg);
                    Ok(emit_pos_then(pos, instr))
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

fn emit_value_only_collection<F: FnOnce(isize) -> InstructLitConst>(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    fields: &[tast::Afield],
    constructor: F,
) -> Result {
    let limit = *(e.options().max_array_elem_size_on_the_stack.get()) as usize;
    let inline = |e: &mut Emitter, exprs: &[tast::Afield]| -> Result {
        Ok(InstrSeq::gather(vec![
            InstrSeq::gather(
                exprs
                    .iter()
                    .map(|f| emit_expr(e, env, f.value()))
                    .collect::<Result<_>>()?,
            ),
            emit_pos(pos),
            instr::lit_const(constructor(exprs.len() as isize)),
        ]))
    };
    let outofline = |e: &mut Emitter, exprs: &[tast::Afield]| -> Result {
        Ok(InstrSeq::gather(
            exprs
                .iter()
                .map(|f| {
                    Ok(InstrSeq::gather(vec![
                        emit_expr(e, env, f.value())?,
                        instr::add_new_elemc(),
                    ]))
                })
                .collect::<Result<_>>()?,
        ))
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

fn emit_record(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    (cid, es): &(tast::Sid, Vec<(tast::Expr, tast::Expr)>),
) -> Result {
    let es = mk_afkvalues(es);
    let id = class::Type::from_ast_name_and_mangle(&cid.1);
    emit_symbol_refs::add_class(e, id.clone());
    emit_struct_array(e, env, pos, &es, |_, keys| Ok(instr::new_record(id, keys)))
}

fn emit_call_isset_expr(e: &mut Emitter, env: &Env, outer_pos: &Pos, expr: &tast::Expr) -> Result {
    let pos = &expr.0;
    if let Some((base_expr, opt_elem_expr)) = expr.1.as_array_get() {
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
    if let Some((cid, id, _)) = expr.1.as_class_get() {
        return emit_class_get(e, env, QueryOp::Isset, cid, id);
    }
    if let Some((expr_, prop, nullflavor, _)) = expr.1.as_obj_get() {
        return Ok(emit_obj_get(e, env, pos, QueryOp::Isset, expr_, prop, nullflavor, false)?.0);
    }
    if let Some(lid) = expr.1.as_lvar() {
        let name = local_id::get_name(&lid.1);
        return Ok(if superglobals::is_any_global(&name) {
            InstrSeq::gather(vec![
                emit_pos(outer_pos),
                instr::string(string_utils::locals::strip_dollar(&name)),
                emit_pos(outer_pos),
                instr::issetg(),
            ])
        } else if is_local_this(env, &lid.1) && !env.flags.contains(env::Flags::NEEDS_LOCAL_THIS) {
            InstrSeq::gather(vec![
                emit_pos(outer_pos),
                emit_local(e, env, BareThisOp::NoNotice, lid)?,
                emit_pos(outer_pos),
                instr::istypec(IstypeOp::OpNull),
                instr::not(),
            ])
        } else {
            emit_pos_then(outer_pos, instr::issetl(get_local(e, env, &lid.0, name)?))
        });
    }
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        instr::istypec(IstypeOp::OpNull),
        instr::not(),
    ]))
}

fn emit_call_isset_exprs(e: &mut Emitter, env: &Env, pos: &Pos, exprs: &[tast::Expr]) -> Result {
    match exprs {
        [] => Err(emit_fatal::raise_fatal_parse(
            pos,
            "Cannot use isset() without any arguments",
        )),
        [expr] => emit_call_isset_expr(e, env, pos, expr),
        _ => {
            let its_done = e.label_gen_mut().next_regular();
            Ok(InstrSeq::gather(vec![
                InstrSeq::gather(
                    exprs
                        .iter()
                        .enumerate()
                        .map(|(i, expr)| {
                            Ok(InstrSeq::gather(vec![
                                emit_call_isset_expr(e, env, pos, expr)?,
                                if i < exprs.len() - 1 {
                                    InstrSeq::gather(vec![
                                        instr::dup(),
                                        instr::jmpz(its_done.clone()),
                                        instr::popc(),
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

fn emit_tag_provenance_here(e: &mut Emitter, env: &Env, pos: &Pos, es: &[tast::Expr]) -> Result {
    let pop = if es.len() == 1 {
        instr::empty()
    } else {
        instr::popc()
    };
    Ok(InstrSeq::gather(vec![
        emit_exprs(e, env, es)?,
        emit_pos(pos),
        pop,
    ]))
}

fn emit_array_mark_legacy(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    es: &[tast::Expr],
    legacy: bool,
) -> Result {
    let default = if es.len() == 1 {
        instr::false_()
    } else {
        instr::empty()
    };
    let mark = if legacy {
        instr::instr(Instruct::IMisc(InstructMisc::ArrayMarkLegacy))
    } else {
        instr::instr(Instruct::IMisc(InstructMisc::ArrayUnmarkLegacy))
    };
    Ok(InstrSeq::gather(vec![
        emit_exprs(e, env, es)?,
        emit_pos(pos),
        default,
        mark,
    ]))
}

fn emit_idx(e: &mut Emitter, env: &Env, pos: &Pos, es: &[tast::Expr]) -> Result {
    let default = if es.len() == 2 {
        instr::null()
    } else {
        instr::empty()
    };
    Ok(InstrSeq::gather(vec![
        emit_exprs(e, env, es)?,
        emit_pos(pos),
        default,
        instr::idx(),
    ]))
}

fn emit_call(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    expr: &tast::Expr,
    targs: &[tast::Targ],
    args: &[tast::Expr],
    uarg: Option<&tast::Expr>,
    async_eager_label: Option<Label>,
) -> Result {
    if let Some(ast_defs::Id(_, s)) = expr.as_id() {
        let fid = function::Type::from_ast_name(s);
        emit_symbol_refs::add_function(e, fid);
    }
    let fcall_args = get_fcall_args(
        args,
        uarg,
        async_eager_label,
        env.call_context.clone(),
        false,
    );
    match expr.1.as_id() {
        None => emit_call_default(e, env, pos, expr, targs, args, uarg, fcall_args),
        Some(ast_defs::Id(_, id)) => {
            let fq = function::Type::from_ast_name(id);
            let lower_fq_name = fq.to_raw_string();
            emit_special_function(e, env, pos, args, uarg, lower_fq_name)
                .transpose()
                .unwrap_or_else(|| {
                    emit_call_default(e, env, pos, expr, targs, args, uarg, fcall_args)
                })
        }
    }
}

fn emit_call_default(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    expr: &tast::Expr,
    targs: &[tast::Targ],
    args: &[tast::Expr],
    uarg: Option<&tast::Expr>,
    fcall_args: FcallArgs,
) -> Result {
    scope::with_unnamed_locals(e, |e| {
        let FcallArgs(_, _, num_ret, _, _, _) = &fcall_args;
        let num_uninit = num_ret - 1;
        let (lhs, fcall) = emit_call_lhs_and_fcall(e, env, expr, fcall_args, targs)?;
        let (args, inout_setters) = emit_args_inout_setters(e, env, args)?;
        let uargs = uarg.map_or(Ok(instr::empty()), |uarg| emit_expr(e, env, uarg))?;
        Ok((
            instr::empty(),
            InstrSeq::gather(vec![
                InstrSeq::gather(
                    iter::repeat(instr::nulluninit())
                        .take(num_uninit)
                        .collect::<Vec<_>>(),
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

pub fn emit_reified_targs(e: &mut Emitter, env: &Env, pos: &Pos, targs: &[&tast::Hint]) -> Result {
    let current_fun_tparams = env.scope.get_fun_tparams();
    let current_cls_tparams = env.scope.get_class_tparams();
    let is_in_lambda = env.scope.is_in_lambda();
    let is_soft =
        |ual: &Vec<tast::UserAttribute>| ual.iter().any(|ua| user_attributes::is_soft(&ua.name.1));
    let same_as_targs = |tparams: &[tast::Tparam]| {
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
        instr::cgetl(local::Type::Named(
            string_utils::reified::GENERICS_LOCAL_NAME.into(),
        ))
    } else if !is_in_lambda && same_as_targs(&current_cls_tparams[..]) {
        InstrSeq::gather(vec![
            instr::checkthis(),
            instr::baseh(),
            instr::querym(
                0,
                QueryOp::CGet,
                MemberKey::PT(
                    prop::from_raw_string(string_utils::reified::PROP_NAME),
                    ReadOnlyOp::Any,
                ),
            ),
        ])
    } else {
        let instrs = InstrSeq::gather(vec![
            InstrSeq::gather(
                targs
                    .iter()
                    .map(|h| Ok(emit_reified_arg(e, env, pos, false, h)?.0))
                    .collect::<Result<Vec<_>>>()?,
            ),
            instr::new_vec_array(targs.len() as isize),
        ]);
        instrs
    })
}

fn get_erased_tparams<'a>(env: &'a Env<'a>) -> Vec<&'a str> {
    env.scope
        .get_tparams()
        .iter()
        .filter_map(|tparam| match tparam.reified {
            tast::ReifyKind::Erased => Some(tparam.name.1.as_str()),
            _ => None,
        })
        .collect()
}

pub fn has_non_tparam_generics(env: &Env, hints: &[tast::Hint]) -> bool {
    let erased_tparams = get_erased_tparams(env);
    hints.iter().any(|hint| {
        hint.1
            .as_happly()
            .map_or(true, |(id, _)| !erased_tparams.contains(&id.1.as_str()))
    })
}

fn has_non_tparam_generics_targs(env: &Env, targs: &[tast::Targ]) -> bool {
    let erased_tparams = get_erased_tparams(env);
    targs.iter().any(|targ| {
        (targ.1)
            .1
            .as_happly()
            .map_or(true, |(id, _)| !erased_tparams.contains(&id.1.as_str()))
    })
}

fn from_ast_null_flavor(nullflavor: tast::OgNullFlavor) -> ObjNullFlavor {
    match nullflavor {
        tast::OgNullFlavor::OGNullsafe => ObjNullFlavor::NullSafe,
        tast::OgNullFlavor::OGNullthrows => ObjNullFlavor::NullThrows,
    }
}

fn emit_object_expr(e: &mut Emitter, env: &Env, expr: &tast::Expr) -> Result {
    match &expr.1 {
        tast::Expr_::Lvar(x) if is_local_this(env, &x.1) => Ok(instr::this()),
        _ => emit_expr(e, env, expr),
    }
}

fn emit_call_lhs_and_fcall(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    mut fcall_args: FcallArgs,
    targs: &[tast::Targ],
) -> Result<(InstrSeq, InstrSeq)> {
    let tast::Expr(pos, expr_) = expr;
    use tast::{Expr as E, Expr_ as E_};

    let emit_generics = |e: &mut Emitter, env, fcall_args: &mut FcallArgs| {
        let does_not_have_non_tparam_generics = !has_non_tparam_generics_targs(env, targs);
        if does_not_have_non_tparam_generics {
            Ok(instr::empty())
        } else {
            *(&mut fcall_args.0) = fcall_args.0 | FcallFlags::HAS_GENERICS;
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
        e: &mut Emitter,
        env,
        expr: &tast::Expr,
        fcall_args: FcallArgs,
    | -> Result<(InstrSeq, InstrSeq)> {
        let tmp = e.local_gen_mut().get_unnamed();
        Ok((
            InstrSeq::gather(vec![
                instr::nulluninit(),
                instr::nulluninit(),
                emit_expr(e, env, expr)?,
                instr::popl(tmp.clone()),
            ]),
            InstrSeq::gather(vec![instr::pushl(tmp), instr::fcallfunc(fcall_args)]),
        ))
    };

    match expr_ {
        E_::ObjGet(o) => {
            if o.as_ref().3 {
                // Case ($x->foo)(...).
                let expr = E(
                    pos.clone(),
                    E_::ObjGet(Box::new((o.0.clone(), o.1.clone(), o.2.clone(), false))),
                );
                emit_fcall_func(e, env, &expr, fcall_args)
            } else {
                // Case $x->foo(...).
                let emit_id =
                    |e: &mut Emitter, obj, id, null_flavor: &tast::OgNullFlavor, mut fcall_args| {
                        // TODO(hrust): enable let name = method::Type::from_ast_name(id);
                        let name: method::Type =
                            string_utils::strip_global_ns(id).to_string().into();
                        let obj = emit_object_expr(e, env, obj)?;
                        let generics = emit_generics(e, env, &mut fcall_args)?;
                        let null_flavor = from_ast_null_flavor(*null_flavor);
                        Ok((
                            InstrSeq::gather(vec![obj, instr::nulluninit()]),
                            InstrSeq::gather(vec![
                                generics,
                                instr::fcallobjmethodd(fcall_args, name, null_flavor),
                            ]),
                        ))
                    };
                match o.as_ref() {
                    (obj, E(_, E_::String(id)), null_flavor, _) => {
                        emit_id(
                            e,
                            obj,
                            // FIXME: This is not safe--string literals are binary strings.
                            // There's no guarantee that they're valid UTF-8.
                            unsafe { std::str::from_utf8_unchecked(id.as_slice().into()) },
                            null_flavor,
                            fcall_args,
                        )
                    }
                    (E(pos, E_::New(new_exp)), E(_, E_::Id(id)), null_flavor, _)
                        if fcall_args.1 == 0 =>
                    {
                        let cexpr = ClassExpr::class_id_to_class_expr(
                            e, false, false, &env.scope, &new_exp.0,
                        );
                        match &cexpr {
                            ClassExpr::Id(ast_defs::Id(_, name))
                                if string_utils::strip_global_ns(name) == "ReflectionClass" =>
                            {
                                let fid = match string_utils::strip_global_ns(&id.1) {
                                    "isAbstract" => {
                                        Some("__SystemLib\\reflection_class_is_abstract")
                                    }
                                    "isInterface" => {
                                        Some("__SystemLib\\reflection_class_is_interface")
                                    }
                                    "isFinal" => Some("__SystemLib\\reflection_class_is_final"),
                                    "getName" => Some("__SystemLib\\reflection_class_get_name"),
                                    _ => None,
                                };
                                match fid {
                                    None => {
                                        emit_id(e, &o.as_ref().0, &id.1, null_flavor, fcall_args)
                                    }
                                    Some(fid) => {
                                        let fcall_args = FcallArgs::new(
                                            FcallFlags::default(),
                                            1,
                                            vec![],
                                            None,
                                            1,
                                            None,
                                        );
                                        let newobj_instrs = emit_new(e, env, pos, &new_exp, true);
                                        Ok((
                                            InstrSeq::gather(vec![
                                                instr::nulluninit(),
                                                instr::nulluninit(),
                                                newobj_instrs?,
                                            ]),
                                            InstrSeq::gather(vec![instr::fcallfuncd(
                                                fcall_args,
                                                function::Type::from_ast_name(fid),
                                            )]),
                                        ))
                                    }
                                }
                            }
                            _ => emit_id(e, &o.as_ref().0, &id.1, null_flavor, fcall_args),
                        }
                    }
                    (obj, E(_, E_::Id(id)), null_flavor, _) => {
                        emit_id(e, obj, &id.1, null_flavor, fcall_args)
                    }
                    (obj, method_expr, null_flavor, _) => {
                        let obj = emit_object_expr(e, env, obj)?;
                        let tmp = e.local_gen_mut().get_unnamed();
                        let null_flavor = from_ast_null_flavor(*null_flavor);
                        Ok((
                            InstrSeq::gather(vec![
                                obj,
                                instr::nulluninit(),
                                emit_expr(e, env, method_expr)?,
                                instr::popl(tmp.clone()),
                            ]),
                            InstrSeq::gather(vec![
                                instr::pushl(tmp),
                                instr::fcallobjmethod(fcall_args, null_flavor),
                            ]),
                        ))
                    }
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
            // TODO(hrust) enabel `let method_id = method::Type::from_ast_name(&id);`,
            // `from_ast_name` should be able to accpet Cow<str>
            let method_id: method::Type = string_utils::strip_global_ns(&id).to_string().into();
            Ok(match cexpr {
                // Statically known
                ClassExpr::Id(ast_defs::Id(_, cname)) => {
                    let cid = class::Type::from_ast_name_and_mangle(&cname);
                    emit_symbol_refs::add_class(e, cid.clone());
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(vec![instr::nulluninit(), instr::nulluninit()]),
                        InstrSeq::gather(vec![
                            generics,
                            instr::fcallclsmethodd(fcall_args, method_id, cid),
                        ]),
                    )
                }
                ClassExpr::Special(clsref) => {
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(vec![instr::nulluninit(), instr::nulluninit()]),
                        InstrSeq::gather(vec![
                            generics,
                            instr::fcallclsmethodsd(fcall_args, clsref, method_id),
                        ]),
                    )
                }
                ClassExpr::Expr(expr) => {
                    let generics = emit_generics(e, env, &mut fcall_args)?;
                    (
                        InstrSeq::gather(vec![instr::nulluninit(), instr::nulluninit()]),
                        InstrSeq::gather(vec![
                            generics,
                            instr::string(method_id.to_raw_string()),
                            emit_expr(e, env, &expr)?,
                            instr::classgetc(),
                            instr::fcallclsmethod(
                                IsLogAsDynamicCallOp::DontLogAsDynamicCall,
                                fcall_args,
                            ),
                        ]),
                    )
                }
                ClassExpr::Reified(instrs) => {
                    let tmp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            instr::nulluninit(),
                            instr::nulluninit(),
                            instrs,
                            instr::popl(tmp.clone()),
                        ]),
                        InstrSeq::gather(vec![
                            instr::string(method_id.to_raw_string()),
                            instr::pushl(tmp),
                            instr::classgetc(),
                            instr::fcallclsmethod(
                                IsLogAsDynamicCallOp::LogAsDynamicCall,
                                fcall_args,
                            ),
                        ]),
                    )
                }
            })
        }
        E_::ClassGet(c) => {
            if c.as_ref().2 {
                // Case (Foo::$bar)(...).
                let expr = E(
                    pos.clone(),
                    E_::ClassGet(Box::new((c.0.clone(), c.1.clone(), false))),
                );
                emit_fcall_func(e, env, &expr, fcall_args)
            } else {
                // Case Foo::bar(...).
                let (cid, cls_get_expr, _) = &**c;
                let mut cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
                if let ClassExpr::Id(ast_defs::Id(_, name)) = &cexpr {
                    if let Some(reified_var_cexpr) = get_reified_var_cexpr(env, pos, &name)? {
                        cexpr = reified_var_cexpr;
                    }
                }
                let emit_meth_name = |e: &mut Emitter| match &cls_get_expr {
                    tast::ClassGetExpr::CGstring((pos, id)) => Ok(emit_pos_then(
                        pos,
                        instr::cgetl(local::Type::Named(id.clone())),
                    )),
                    tast::ClassGetExpr::CGexpr(expr) => emit_expr(e, env, expr),
                };
                Ok(match cexpr {
                    ClassExpr::Id(cid) => {
                        let tmp = e.local_gen_mut().get_unnamed();
                        (
                            InstrSeq::gather(vec![
                                instr::nulluninit(),
                                instr::nulluninit(),
                                emit_meth_name(e)?,
                                instr::popl(tmp.clone()),
                            ]),
                            InstrSeq::gather(vec![
                                instr::pushl(tmp),
                                emit_known_class_id(e, &cid),
                                instr::fcallclsmethod(
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
                                instr::nulluninit(),
                                instr::nulluninit(),
                                emit_meth_name(e)?,
                                instr::popl(tmp.clone()),
                            ]),
                            InstrSeq::gather(vec![
                                instr::pushl(tmp),
                                instr::fcallclsmethods(fcall_args, clsref),
                            ]),
                        )
                    }
                    ClassExpr::Expr(expr) => {
                        let cls = e.local_gen_mut().get_unnamed();
                        let meth = e.local_gen_mut().get_unnamed();
                        (
                            InstrSeq::gather(vec![
                                instr::nulluninit(),
                                instr::nulluninit(),
                                emit_expr(e, env, &expr)?,
                                instr::popl(cls.clone()),
                                emit_meth_name(e)?,
                                instr::popl(meth.clone()),
                            ]),
                            InstrSeq::gather(vec![
                                instr::pushl(meth),
                                instr::pushl(cls),
                                instr::classgetc(),
                                instr::fcallclsmethod(
                                    IsLogAsDynamicCallOp::LogAsDynamicCall,
                                    fcall_args,
                                ),
                            ]),
                        )
                    }
                    ClassExpr::Reified(instrs) => {
                        let cls = e.local_gen_mut().get_unnamed();
                        let meth = e.local_gen_mut().get_unnamed();
                        (
                            InstrSeq::gather(vec![
                                instr::nulluninit(),
                                instr::nulluninit(),
                                instrs,
                                instr::popl(cls.clone()),
                                emit_meth_name(e)?,
                                instr::popl(meth.clone()),
                            ]),
                            InstrSeq::gather(vec![
                                instr::pushl(meth),
                                instr::pushl(cls),
                                instr::classgetc(),
                                instr::fcallclsmethod(
                                    IsLogAsDynamicCallOp::LogAsDynamicCall,
                                    fcall_args,
                                ),
                            ]),
                        )
                    }
                })
            }
        }
        E_::Id(id) => {
            let FcallArgs(flags, num_args, _, _, _, _) = fcall_args;
            let fq_id = match string_utils::strip_global_ns(&id.1) {
                "min" if num_args == 2 && !flags.contains(FcallFlags::HAS_UNPACK) => {
                    function::Type::from_ast_name("__SystemLib\\min2")
                }
                "max" if num_args == 2 && !flags.contains(FcallFlags::HAS_UNPACK) => {
                    function::Type::from_ast_name("__SystemLib\\max2")
                }
                _ => {
                    //TODO(hrust): enable `function::Type::from_ast_name(&id.1)`
                    string_utils::strip_global_ns(&id.1).to_string().into()
                }
            };
            let generics = emit_generics(e, env, &mut fcall_args)?;
            Ok((
                InstrSeq::gather(vec![instr::nulluninit(), instr::nulluninit()]),
                InstrSeq::gather(vec![generics, instr::fcallfuncd(fcall_args, fq_id)]),
            ))
        }
        E_::String(s) => {
            // TODO(hrust) should be able to accept `let fq_id = function::from_raw_string(s);`
            let fq_id = s.to_string().into();
            let generics = emit_generics(e, env, &mut fcall_args)?;
            Ok((
                InstrSeq::gather(vec![instr::nulluninit(), instr::nulluninit()]),
                InstrSeq::gather(vec![generics, instr::fcallfuncd(fcall_args, fq_id)]),
            ))
        }
        _ => emit_fcall_func(e, env, expr, fcall_args),
    }
}

fn get_reified_var_cexpr(env: &Env, pos: &Pos, name: &str) -> Result<Option<ClassExpr>> {
    Ok(emit_reified_type_opt(env, pos, name)?.map(|instrs| {
        ClassExpr::Reified(InstrSeq::gather(vec![
            instrs,
            instr::basec(0, MemberOpMode::Warn),
            instr::querym(
                1,
                QueryOp::CGet,
                MemberKey::ET("classname".into(), ReadOnlyOp::Any),
            ),
        ]))
    }))
}

fn emit_args_inout_setters(
    e: &mut Emitter,
    env: &Env,
    args: &[tast::Expr],
) -> Result<(InstrSeq, InstrSeq)> {
    let aliases = if has_inout_arg(args) {
        inout_locals::collect_written_variables(env, args)
    } else {
        inout_locals::AliasInfoMap::new()
    };
    fn emit_arg_and_inout_setter(
        e: &mut Emitter,
        env: &Env,
        i: usize,
        arg: &tast::Expr,
        aliases: &inout_locals::AliasInfoMap,
    ) -> Result<(InstrSeq, InstrSeq)> {
        use tast::Expr_ as E_;
        match &arg.1 {
            E_::Callconv(cc) if (cc.0).is_pinout() => {
                match &(cc.1).1 {
                    // inout $var
                    E_::Lvar(l) => {
                        let local = get_local(e, env, &l.0, local_id::get_name(&l.1))?;
                        let move_instrs = if !env.flags.contains(env::Flags::IN_TRY)
                            && inout_locals::should_move_local_value(&local, aliases)
                        {
                            InstrSeq::gather(vec![instr::null(), instr::popl(local.clone())])
                        } else {
                            instr::empty()
                        };
                        Ok((
                            InstrSeq::gather(vec![instr::cgetl(local.clone()), move_instrs]),
                            instr::popl(local),
                        ))
                    }
                    // inout $arr[...][...]
                    E_::ArrayGet(ag) => {
                        let array_get_result = emit_array_get_(
                            e,
                            env,
                            &(cc.1).0,
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
                                    &(cc.1).0,
                                    Some(MemberOpMode::Define),
                                    QueryOp::InOut,
                                    &ag.0,
                                    ag.1.as_ref(),
                                    true,
                                    false,
                                )?
                                .0;
                                let (mk, warninstr) =
                                    get_elem_member_key(e, env, 0, ag.1.as_ref(), false)?;
                                let setter = InstrSeq::gather(vec![
                                    warninstr,
                                    setter_base,
                                    instr::setm(0, mk),
                                    instr::popc(),
                                ]);
                                (instrs, setter)
                            }
                            ArrayGetInstr::Inout { load, store } => {
                                let (mut ld, mut st) = (vec![], vec![store]);
                                for (instr, local_kind_opt) in load.into_iter() {
                                    match local_kind_opt {
                                        None => ld.push(instr),
                                        Some((l, kind)) => {
                                            let unset = instr::unsetl(l.clone());
                                            let set = match kind {
                                                StoredValueKind::Expr => instr::setl(l),
                                                _ => instr::popl(l),
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
                    _ => Err(unrecoverable(
                        "emit_arg_and_inout_setter: Unexpected inout expression type",
                    )),
                }
            }
            _ => Ok((emit_expr(e, env, arg)?, instr::empty())),
        }
    }
    let (instr_args, instr_setters): (Vec<InstrSeq>, Vec<InstrSeq>) = args
        .iter()
        .enumerate()
        .map(|(i, arg)| emit_arg_and_inout_setter(e, env, i, arg, &aliases))
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
                instr::popl(retval.clone()),
                instr_setters,
                instr::pushl(retval),
            ]),
        ))
    } else {
        Ok((instr_args, instr::empty()))
    }
}

fn get_fcall_args(
    args: &[tast::Expr],
    uarg: Option<&tast::Expr>,
    async_eager_label: Option<Label>,
    context: Option<String>,
    lock_while_unwinding: bool,
) -> FcallArgs {
    let num_args = args.len();
    let num_rets = 1 + args.iter().filter(|x| is_inout_arg(*x)).count();
    let mut flags = FcallFlags::default();
    flags.set(FcallFlags::HAS_UNPACK, uarg.is_some());
    flags.set(FcallFlags::LOCK_WHILE_UNWINDING, lock_while_unwinding);
    let inouts: Vec<bool> = args.iter().map(is_inout_arg).collect();
    FcallArgs::new(
        flags,
        num_rets,
        inouts,
        async_eager_label,
        num_args,
        context,
    )
}

fn is_inout_arg(e: &tast::Expr) -> bool {
    e.1.as_callconv().map_or(false, |cc| cc.0.is_pinout())
}

fn has_inout_arg(es: &[tast::Expr]) -> bool {
    es.iter().any(is_inout_arg)
}

fn emit_special_function(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    args: &[tast::Expr],
    uarg: Option<&tast::Expr>,
    lower_fq_name: &str,
) -> Result<Option<InstrSeq>> {
    use tast::{Expr as E, Expr_ as E_};
    let nargs = args.len() + uarg.map_or(0, |_| 1);
    let fun_and_clsmeth_disabled = e
        .options()
        .hhvm
        .hack_lang
        .flags
        .contains(LangFlags::DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS);
    match (lower_fq_name, args) {
        (id, _) if id == special_functions::ECHO => Ok(Some(InstrSeq::gather(
            args.iter()
                .enumerate()
                .map(|(i, arg)| {
                    Ok(InstrSeq::gather(vec![
                        emit_expr(e, env, arg)?,
                        emit_pos(pos),
                        instr::print(),
                        if i == nargs - 1 {
                            instr::empty()
                        } else {
                            instr::popc()
                        },
                    ]))
                })
                .collect::<Result<_>>()?,
        ))),
        ("unsafe_cast", &[]) => Ok(Some(instr::null())),
        ("unsafe_cast", args) => Ok(Some(emit_expr(e, env, &args[0])?)),

        ("HH\\invariant", args) if args.len() >= 2 => {
            let l = e.label_gen_mut().next_regular();
            let expr_id = tast::Expr(
                pos.clone(),
                tast::Expr_::mk_id(ast_defs::Id(
                    pos.clone(),
                    "\\hh\\invariant_violation".into(),
                )),
            );
            let call = tast::Expr(
                pos.clone(),
                tast::Expr_::mk_call(expr_id, vec![], args[1..].to_owned(), uarg.cloned()),
            );
            let ignored_expr = emit_ignored_expr(e, env, &Pos::make_none(), &call)?;
            Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, &args[0])?,
                instr::jmpnz(l.clone()),
                ignored_expr,
                emit_fatal::emit_fatal_runtime(pos, "invariant_violation"),
                instr::label(l),
                instr::null(),
            ])))
        }
        ("HH\\sequence", &[]) => Ok(Some(instr::null())),
        ("HH\\sequence", args) => Ok(Some(InstrSeq::gather(
            args.iter()
                .map(|arg| emit_expr(e, env, arg))
                .collect::<Result<Vec<_>>>()?
                .into_iter()
                .intersperse(instr::popc())
                .collect::<Vec<_>>(),
        ))),
        ("class_exists", &[ref arg1, ..])
        | ("trait_exists", &[ref arg1, ..])
        | ("interface_exists", &[ref arg1, ..])
            if nargs == 1 || nargs == 2 =>
        {
            let class_kind = match lower_fq_name {
                "class_exists" => ClassKind::Class,
                "interface_exists" => ClassKind::Interface,
                "trait_exists" => ClassKind::Trait,
                _ => return Err(unrecoverable("emit_special_function: class_kind")),
            };
            Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, arg1)?,
                instr::cast_string(),
                if nargs == 1 {
                    instr::true_()
                } else {
                    InstrSeq::gather(vec![emit_expr(e, env, &args[1])?, instr::cast_bool()])
                },
                instr::oodeclexists(class_kind),
            ])))
        }
        ("exit", _) | ("die", _) if nargs == 0 || nargs == 1 => {
            Ok(Some(emit_exit(e, env, args.first())?))
        }
        ("HH\\fun", _) => {
            if fun_and_clsmeth_disabled {
                match args {
                    [tast::Expr(_, tast::Expr_::String(func_name))] => {
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
                    [tast::Expr(_, tast::Expr_::String(func_name))] => {
                        Ok(Some(emit_hh_fun(
                            e,
                            env,
                            pos,
                            &vec![ /* targs */ ],
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
                [E(_, E_::String(ref func_name))] => Ok(Some(instr::resolve_meth_caller(
                    // TODO(hrust) should accept functions::from_raw_string(func_name)
                    string_utils::strip_global_ns(
                        // FIXME: This is not safe--string literals are binary strings.
                        // There's no guarantee that they're valid UTF-8.
                        unsafe { std::str::from_utf8_unchecked(func_name.as_slice().into()) },
                    )
                    .to_string()
                    .into(),
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
                    [E(_, E_::Lvar(id))] => {
                        Ok(Some(instr::isunsetl(get_local(e, env, pos, id.name())?)))
                    }
                    _ => Err(emit_fatal::raise_fatal_runtime(
                        pos,
                        "Local variable expected in __debugger_is_uninit()",
                    )),
                }
            }
        }
        ("__SystemLib\\get_enum_member_by_label", _) if e.systemlib() => {
            let local = match args {
                [E(_, E_::Lvar(id))] => get_local(e, env, pos, id.name()),
                _ => Err(emit_fatal::raise_fatal_runtime(
                    pos,
                    "Argument must be the label argument",
                )),
            }?;
            Ok(Some(InstrSeq::gather(vec![
                instr::lateboundcls(),
                instr::clscnsl(local),
            ])))
        }
        ("HH\\inst_meth", _) => match args {
            [obj_expr, method_name] => Ok(Some(emit_inst_meth(e, env, obj_expr, method_name)?)),
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
        ("HH\\class_meth", &[ref cls, ref meth, ..]) if nargs == 2 => {
            if meth.1.is_string() {
                if cls.1.is_string()
                    || cls
                        .1
                        .as_class_const()
                        .map_or(false, |(_, (_, id))| string_utils::is_class(id))
                    || cls
                        .1
                        .as_id()
                        .map_or(false, |ast_defs::Id(_, id)| id == pseudo_consts::G__CLASS__)
                {
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
        ("HH\\global_set", _) => match args {
            &[ref gkey, ref gvalue] => Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, gkey)?,
                emit_expr(e, env, gvalue)?,
                emit_pos(pos),
                instr::setg(),
                instr::popc(),
                instr::null(),
            ]))),
            _ => Err(emit_fatal::raise_fatal_runtime(
                pos,
                format!(
                    "global_set() expects exactly 2 parameters, {} given",
                    nargs.to_string()
                ),
            )),
        },
        ("HH\\global_unset", _) => match args {
            &[ref gkey] => Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, gkey)?,
                emit_pos(pos),
                instr::unsetg(),
                instr::null(),
            ]))),
            _ => Err(emit_fatal::raise_fatal_runtime(
                pos,
                format!(
                    "global_unset() expects exactly 1 parameter, {} given",
                    nargs.to_string()
                ),
            )),
        },
        ("__hhvm_internal_whresult", &[E(_, E_::Lvar(ref param))]) if e.systemlib() => {
            Ok(Some(InstrSeq::gather(vec![
                instr::cgetl(local::Type::Named(local_id::get_name(&param.1).into())),
                instr::whresult(),
            ])))
        }
        ("__hhvm_internal_getmemokeyl", &[E(_, E_::Lvar(ref param))]) if e.systemlib() => Ok(Some(
            instr::getmemokeyl(local::Type::Named(local_id::get_name(&param.1).into())),
        )),
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
                    Some(InstrSeq::gather(vec![
                        emit_expr(e, env, &arg_expr)?,
                        is_expr,
                    ]))
                }
                (&[E(_, E_::Lvar(ref arg_id))], Some(i), _)
                    if superglobals::is_any_global(arg_id.name()) =>
                {
                    Some(InstrSeq::gather(vec![
                        emit_local(e, env, BareThisOp::NoNotice, &arg_id)?,
                        emit_pos(pos),
                        instr::istypec(i),
                    ]))
                }
                (&[E(_, E_::Lvar(ref arg_id))], Some(i), _) if !is_local_this(env, &arg_id.1) => {
                    Some(instr::istypel(
                        get_local(e, env, &arg_id.0, &(arg_id.1).1)?,
                        i,
                    ))
                }
                (&[ref arg_expr], Some(i), _) => Some(InstrSeq::gather(vec![
                    emit_expr(e, env, &arg_expr)?,
                    emit_pos(pos),
                    instr::istypec(i),
                ])),
                _ => match get_call_builtin_func_info(lower_fq_name) {
                    Some((nargs, i)) if nargs == args.len() => {
                        let inner = emit_exprs(e, env, args)?;
                        Some(InstrSeq::gather(vec![
                            inner,
                            emit_pos(pos),
                            instr::instr(i),
                        ]))
                    }
                    _ => None,
                },
            },
        ),
    }
}

fn emit_inst_meth(
    e: &mut Emitter,
    env: &Env,
    obj_expr: &tast::Expr,
    method_name: &tast::Expr,
) -> Result {
    let instrs = InstrSeq::gather(vec![
        emit_expr(e, env, obj_expr)?,
        emit_expr(e, env, method_name)?,
        if e.options()
            .hhvm
            .flags
            .contains(HhvmFlags::EMIT_INST_METH_POINTERS)
        {
            instr::resolve_obj_method()
        } else {
            instr::new_vec_array(2)
        },
    ]);
    Ok(instrs)
}

fn emit_class_meth(e: &mut Emitter, env: &Env, cls: &tast::Expr, meth: &tast::Expr) -> Result {
    use tast::Expr_ as E_;
    if e.options()
        .hhvm
        .flags
        .contains(HhvmFlags::EMIT_CLS_METH_POINTERS)
    {
        let method_id = match &meth.1 {
            E_::String(method_name) => method_name.to_string().into(), // TODO(hrust) should accept method::from_raw_string(method_name),
            _ => return Err(unrecoverable("emit_class_meth: unhandled method")),
        };
        if let Some((cid, (_, id))) = cls.1.as_class_const() {
            if string_utils::is_class(id) {
                return emit_class_meth_native(
                    e,
                    env,
                    &cls.0,
                    cid,
                    method_id,
                    &vec![ /* targs */ ],
                );
            }
        }
        if let Some(ast_defs::Id(_, s)) = cls.1.as_id() {
            if s == pseudo_consts::G__CLASS__ {
                return Ok(instr::resolveclsmethods(SpecialClsRef::Self_, method_id));
            }
        }
        if let Some(class_name) = cls.1.as_string() {
            return Ok(instr::resolveclsmethodd(
                // TODO(hrust) should accept class::Type::from_raw_string(class_name)
                string_utils::strip_global_ns(
                    // FIXME: This is not safe--string literals are binary strings.
                    // There's no guarantee that they're valid UTF-8.
                    unsafe { std::str::from_utf8_unchecked(class_name.as_slice().into()) },
                )
                .to_string()
                .into(),
                method_id,
            ));
        }
        Err(unrecoverable("emit_class_meth: unhandled method"))
    } else {
        let instrs = InstrSeq::gather(vec![
            emit_expr(e, env, cls)?,
            emit_expr(e, env, meth)?,
            instr::new_vec_array(2),
        ]);
        Ok(instrs)
    }
}

fn emit_class_meth_native(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    cid: &tast::ClassId,
    method_id: MethodId,
    targs: &[tast::Targ],
) -> Result {
    let mut cexpr = ClassExpr::class_id_to_class_expr(e, false, true, &env.scope, cid);
    if let ClassExpr::Id(ast_defs::Id(_, name)) = &cexpr {
        if let Some(reified_var_cexpr) = get_reified_var_cexpr(env, pos, &name)? {
            cexpr = reified_var_cexpr;
        }
    }
    let has_generics = has_non_tparam_generics_targs(env, targs);
    let mut emit_generics = || -> Result {
        emit_reified_targs(
            e,
            env,
            pos,
            &targs.iter().map(|targ| &targ.1).collect::<Vec<_>>(),
        )
    };
    Ok(match cexpr {
        ClassExpr::Id(ast_defs::Id(_, name)) if !has_generics => {
            instr::resolveclsmethodd(class::Type::from_ast_name_and_mangle(&name), method_id)
        }
        ClassExpr::Id(ast_defs::Id(_, name)) => InstrSeq::gather(vec![
            emit_generics()?,
            instr::resolverclsmethodd(class::Type::from_ast_name_and_mangle(&name), method_id),
        ]),
        ClassExpr::Special(clsref) if !has_generics => instr::resolveclsmethods(clsref, method_id),
        ClassExpr::Special(clsref) => InstrSeq::gather(vec![
            emit_generics()?,
            instr::resolverclsmethods(clsref, method_id),
        ]),
        ClassExpr::Reified(instrs) if !has_generics => InstrSeq::gather(vec![
            instrs,
            instr::classgetc(),
            instr::resolveclsmethod(method_id),
        ]),
        ClassExpr::Reified(instrs) => InstrSeq::gather(vec![
            instrs,
            instr::classgetc(),
            emit_generics()?,
            instr::resolverclsmethod(method_id),
        ]),
        ClassExpr::Expr(_) => {
            return Err(unrecoverable(
                "emit_class_meth_native: ClassExpr::Expr should be impossible",
            ));
        }
    })
}

fn get_call_builtin_func_info(id: impl AsRef<str>) -> Option<(usize, Instruct)> {
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
        "HH\\global_get" => Some((1, IGet(CGetG))),
        "HH\\global_isset" => Some((1, IIsset(IssetG))),
        _ => None,
    }
}

fn emit_function_pointer(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    fpid: &tast::FunctionPtrId,
    targs: &[tast::Targ],
) -> Result {
    let instrs = match fpid {
        // This is a function name. Equivalent to HH\fun('str')
        tast::FunctionPtrId::FPId(id) => emit_hh_fun(e, env, pos, targs, id.name())?,
        // class_meth
        tast::FunctionPtrId::FPClassConst(cid, method_id) => {
            // TODO(hrust) should accept `let method_id = method::Type::from_ast_name(&(cc.1).1);`
            let method_id: method::Type = string_utils::strip_global_ns(&method_id.1)
                .to_string()
                .into();
            emit_class_meth_native(e, env, pos, cid, method_id, targs)?
        }
    };
    Ok(emit_pos_then(pos, instrs))
}

fn emit_hh_fun(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    targs: &[tast::Targ],
    fname: &str,
) -> Result<InstrSeq> {
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
        Ok(InstrSeq::gather(vec![
            generics,
            instr::resolve_rfunc(fname.to_owned().into()),
        ]))
    } else {
        Ok(instr::resolve_func(fname.to_owned().into()))
    }
}

fn emit_is(e: &mut Emitter, env: &Env, pos: &Pos, h: &tast::Hint) -> Result {
    let (ts_instrs, is_static) = emit_reified_arg(e, env, pos, true, h)?;
    Ok(if is_static {
        match &*h.1 {
            aast_defs::Hint_::Happly(ast_defs::Id(_, id), hs)
                if hs.is_empty() && string_utils::strip_hh_ns(&id) == typehints::THIS =>
            {
                instr::islateboundcls()
            }
            _ => InstrSeq::gather(vec![
                get_type_structure_for_hint(e, &[], &IndexSet::new(), h)?,
                instr::is_type_structc_resolve(),
            ]),
        }
    } else {
        InstrSeq::gather(vec![ts_instrs, instr::is_type_structc_dontresolve()])
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

fn is_isexp_op(lower_fq_id: impl AsRef<str>) -> Option<tast::Hint> {
    let h = |s: &str| {
        Some(tast::Hint::new(
            Pos::make_none(),
            tast::Hint_::mk_happly(tast::Id(Pos::make_none(), s.into()), vec![]),
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

fn emit_eval(e: &mut Emitter, env: &Env, pos: &Pos, expr: &tast::Expr) -> Result {
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        emit_pos(pos),
        instr::eval(),
    ]))
}

fn has_reified_types(env: &Env) -> bool {
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

fn emit_call_expr(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    async_eager_label: Option<Label>,
    (expr, targs, args, uarg): &(
        tast::Expr,
        Vec<tast::Targ>,
        Vec<tast::Expr>,
        Option<tast::Expr>,
    ),
) -> Result {
    let jit_enable_rename_function = e
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION);
    use {tast::Expr as E, tast::Expr_ as E_};
    match (&expr.1, &args[..], uarg) {
        (E_::Id(id), [E(_, E_::String(data))], None) if id.1 == special_functions::HHAS_ADATA => {
            // FIXME: This is not safe--string literals are binary strings.
            // There's no guarantee that they're valid UTF-8.
            let v =
                TypedValue::HhasAdata(unsafe { String::from_utf8_unchecked(data.clone().into()) });
            Ok(emit_pos_then(pos, instr::typedvalue(v)))
        }
        (E_::Id(id), _, None) if id.1 == pseudo_functions::ISSET => {
            emit_call_isset_exprs(e, env, pos, args)
        }
        (E_::Id(id), args, None)
            if id.1 == fb::IDX
                && !jit_enable_rename_function
                && (args.len() == 2 || args.len() == 3) =>
        {
            emit_idx(e, env, pos, args)
        }
        (E_::Id(id), [arg1], None) if id.1 == emitter_special_functions::EVAL => {
            emit_eval(e, env, pos, arg1)
        }
        (E_::Id(id), [arg1], None) if id.1 == emitter_special_functions::SET_FRAME_METADATA => {
            Ok(InstrSeq::gather(vec![
                emit_expr(e, env, arg1)?,
                emit_pos(pos),
                instr::popl(local::Type::Named("$86metadata".into())),
                instr::null(),
            ]))
        }
        (E_::Id(id), [], None)
            if id.1 == pseudo_functions::EXIT || id.1 == pseudo_functions::DIE =>
        {
            let exit = emit_exit(e, env, None)?;
            Ok(emit_pos_then(pos, exit))
        }
        (E_::Id(id), [arg1], None)
            if id.1 == pseudo_functions::EXIT || id.1 == pseudo_functions::DIE =>
        {
            let exit = emit_exit(e, env, Some(arg1))?;
            Ok(emit_pos_then(pos, exit))
        }
        (E_::Id(id), [], _)
            if id.1 == emitter_special_functions::SYSTEMLIB_REIFIED_GENERICS
                && e.systemlib()
                && has_reified_types(env) =>
        {
            // Rewrite __systemlib_reified_generics() to $0ReifiedGenerics,
            // but only in systemlib functions that take a reified generic.
            let lvar = E::new(
                pos.clone(),
                E_::Lvar(Box::new(tast::Lid(
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
            )?;
            Ok(emit_pos_then(pos, instrs))
        }
    }
}

pub fn emit_reified_generic_instrs(pos: &Pos, is_fun: bool, index: usize) -> Result {
    let base = if is_fun {
        instr::basel(
            local::Type::Named(string_utils::reified::GENERICS_LOCAL_NAME.into()),
            MemberOpMode::Warn,
        )
    } else {
        InstrSeq::gather(vec![
            instr::checkthis(),
            instr::baseh(),
            instr::dim_warn_pt(
                prop::from_raw_string(string_utils::reified::PROP_NAME),
                ReadOnlyOp::Any,
            ),
        ])
    };
    Ok(emit_pos_then(
        pos,
        InstrSeq::gather(vec![
            base,
            instr::querym(
                0,
                QueryOp::CGet,
                MemberKey::EI(index.try_into().unwrap(), ReadOnlyOp::Any),
            ),
        ]),
    ))
}

fn emit_reified_type(env: &Env, pos: &Pos, name: &str) -> Result<InstrSeq> {
    emit_reified_type_opt(env, pos, name)?
        .ok_or_else(|| emit_fatal::raise_fatal_runtime(&Pos::make_none(), "Invalid reified param"))
}

fn emit_reified_type_opt(env: &Env, pos: &Pos, name: &str) -> Result<Option<InstrSeq>> {
    let is_in_lambda = env.scope.is_in_lambda();
    let cget_instr = |is_fun, i| {
        instr::cgetl(local::Type::Named(
            string_utils::reified::reified_generic_captured_name(is_fun, i),
        ))
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
            emit_reified_generic_instrs(pos, is_fun, i)?
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

fn emit_known_class_id(e: &mut Emitter, id: &ast_defs::Id) -> InstrSeq {
    let cid = class::Type::from_ast_name(&id.1);
    let cid_string = instr::string(cid.to_raw_string());
    emit_symbol_refs::add_class(e, cid);
    InstrSeq::gather(vec![cid_string, instr::classgetc()])
}

fn emit_load_class_ref(e: &mut Emitter, env: &Env, pos: &Pos, cexpr: ClassExpr) -> Result {
    let instrs = match cexpr {
        ClassExpr::Special(SpecialClsRef::Self_) => instr::self_(),
        ClassExpr::Special(SpecialClsRef::Static) => instr::lateboundcls(),
        ClassExpr::Special(SpecialClsRef::Parent) => instr::parent(),
        ClassExpr::Id(id) => emit_known_class_id(e, &id),
        ClassExpr::Expr(expr) => InstrSeq::gather(vec![
            emit_pos(pos),
            emit_expr(e, env, &expr)?,
            instr::classgetc(),
        ]),
        ClassExpr::Reified(instrs) => {
            InstrSeq::gather(vec![emit_pos(pos), instrs, instr::classgetc()])
        }
    };
    Ok(emit_pos_then(pos, instrs))
}

fn emit_new(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    (cid, targs, args, uarg, _): &(
        tast::ClassId,
        Vec<tast::Targ>,
        Vec<tast::Expr>,
        Option<tast::Expr>,
        Pos,
    ),
    is_reflection_class_builtin: bool,
) -> Result {
    if has_inout_arg(args) {
        return Err(unrecoverable("Unexpected inout arg in new expr"));
    }
    let resolve_self = match &cid.1.as_ciexpr() {
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
            let (instr_args, _) = emit_args_inout_setters(e, env, args)?;
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
                let id = class::Type::from_ast_name_and_mangle(&cname);
                emit_symbol_refs::add_class(e, id.clone());
                match has_generics {
                    H::NoGenerics => InstrSeq::gather(vec![emit_pos(pos), instr::newobjd(id)]),
                    H::HasGenerics => InstrSeq::gather(vec![
                        emit_pos(pos),
                        emit_reified_targs(
                            e,
                            env,
                            pos,
                            &targs.iter().map(|t| &t.1).collect::<Vec<_>>(),
                        )?,
                        instr::newobjrd(id),
                    ]),
                    H::MaybeGenerics => {
                        return Err(unrecoverable(
                            "Internal error: This case should have been transformed",
                        ));
                    }
                }
            }
            ClassExpr::Special(cls_ref) => {
                InstrSeq::gather(vec![emit_pos(pos), instr::newobjs(cls_ref)])
            }
            ClassExpr::Reified(instrs) if has_generics == H::MaybeGenerics => {
                InstrSeq::gather(vec![instrs, instr::classgetts(), instr::newobjr()])
            }
            _ => InstrSeq::gather(vec![
                emit_load_class_ref(e, env, pos, cexpr)?,
                instr::newobj(),
            ]),
        };
        scope::with_unnamed_locals(e, |e| {
            let (instr_args, _) = emit_args_inout_setters(e, env, args)?;
            let instr_uargs = match uarg {
                None => instr::empty(),
                Some(uarg) => emit_expr(e, env, uarg)?,
            };
            Ok((
                instr::empty(),
                InstrSeq::gather(vec![
                    newobj_instrs,
                    instr::dup(),
                    instr::nulluninit(),
                    instr_args,
                    instr_uargs,
                    emit_pos(pos),
                    instr::fcallctor(get_fcall_args(
                        args,
                        uarg.as_ref(),
                        None,
                        env.call_context.clone(),
                        true,
                    )),
                    instr::popc(),
                    instr::lockobj(),
                ]),
                instr::empty(),
            ))
        })
    }
}

fn emit_obj_get(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    query_op: QueryOp,
    expr: &tast::Expr,
    prop: &tast::Expr,
    nullflavor: &ast_defs::OgNullFlavor,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq, Option<NumParams>)> {
    if let Some(tast::Lid(pos, id)) = expr.1.as_lvar() {
        if local_id::get_name(&id) == special_idents::THIS
            && nullflavor.eq(&ast_defs::OgNullFlavor::OGNullsafe)
        {
            return Err(emit_fatal::raise_fatal_parse(
                pos,
                "?-> is not allowed with $this",
            ));
        }
    }
    if let Some(ast_defs::Id(_, s)) = prop.1.as_id() {
        if string_utils::is_xhp(s) {
            return Ok((emit_xhp_obj_get(e, env, pos, &expr, s, nullflavor)?, None));
        }
    }
    let mode = if null_coalesce_assignment {
        MemberOpMode::Warn
    } else {
        get_querym_op_mode(&query_op)
    };
    let prop_stack_size = emit_prop_expr(e, env, nullflavor, 0, prop, null_coalesce_assignment)?.2;
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
    )?;
    let (mk, prop_instrs, _) = emit_prop_expr(
        e,
        env,
        nullflavor,
        cls_stack_size,
        prop,
        null_coalesce_assignment,
    )?;
    let total_stack_size = (prop_stack_size + base_stack_size + cls_stack_size) as usize;
    let num_params = if null_coalesce_assignment {
        0
    } else {
        total_stack_size
    };
    let final_instr = instr::querym(num_params, query_op, mk);
    let querym_n_unpopped = if null_coalesce_assignment {
        Some(total_stack_size)
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
fn emit_prop_expr(
    e: &mut Emitter,
    env: &Env,
    nullflavor: &ast_defs::OgNullFlavor,
    stack_index: StackIndex,
    prop: &tast::Expr,
    null_coalesce_assignment: bool,
) -> Result<(MemberKey, InstrSeq, StackIndex)> {
    let mk = match &prop.1 {
        tast::Expr_::Id(id) => {
            let ast_defs::Id(pos, name) = &**id;
            if name.starts_with("$") {
                MemberKey::PL(get_local(e, env, pos, name)?, ReadOnlyOp::Any)
            } else {
                // Special case for known property name

                // TODO(hrust) enable `let pid = prop::Type::from_ast_name(name);`,
                // `from_ast_name` should be able to accpet Cow<str>
                let pid: prop::Type = string_utils::strip_global_ns(&name).to_string().into();
                match nullflavor {
                    ast_defs::OgNullFlavor::OGNullthrows => MemberKey::PT(pid, ReadOnlyOp::Any),
                    ast_defs::OgNullFlavor::OGNullsafe => MemberKey::QT(pid, ReadOnlyOp::Any),
                }
            }
        }
        // Special case for known property name
        tast::Expr_::String(name) => {
            // TODO(hrust) enable `let pid = prop::Type::from_ast_name(name);`,
            // `from_ast_name` should be able to accpet Cow<str>
            let pid: prop::Type = string_utils::strip_global_ns(
                // FIXME: This is not safe--string literals are binary strings.
                // There's no guarantee that they're valid UTF-8.
                unsafe { std::str::from_utf8_unchecked(name.as_slice().into()) },
            )
            .to_string()
            .into();
            match nullflavor {
                ast_defs::OgNullFlavor::OGNullthrows => MemberKey::PT(pid, ReadOnlyOp::Any),
                ast_defs::OgNullFlavor::OGNullsafe => MemberKey::QT(pid, ReadOnlyOp::Any),
            }
        }
        tast::Expr_::Lvar(lid) if !(is_local_this(env, &lid.1)) => MemberKey::PL(
            get_local(e, env, &lid.0, local_id::get_name(&lid.1))?,
            ReadOnlyOp::Any,
        ),
        _ => {
            // General case
            MemberKey::PC(stack_index, ReadOnlyOp::Any)
        }
    };
    // For nullsafe access, insist that property is known
    Ok(match mk {
        MemberKey::PL(_, _) | MemberKey::PC(_, _)
            if nullflavor.eq(&ast_defs::OgNullFlavor::OGNullsafe) =>
        {
            return Err(emit_fatal::raise_fatal_parse(
                &prop.0,
                "?-> can only be used with scalar property names",
            ));
        }
        MemberKey::PC(_, _) => (mk, emit_expr(e, env, prop)?, 1),
        MemberKey::PL(local, ReadOnlyOp::Any) if null_coalesce_assignment => (
            MemberKey::PC(stack_index, ReadOnlyOp::Any),
            instr::cgetl(local),
            1,
        ),
        _ => (mk, instr::empty(), 0),
    })
}

fn emit_xhp_obj_get(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    expr: &tast::Expr,
    s: &str,
    nullflavor: &ast_defs::OgNullFlavor,
) -> Result {
    use tast::Expr as E;
    use tast::Expr_ as E_;
    let f = E(
        pos.clone(),
        E_::mk_obj_get(
            expr.clone(),
            E(
                pos.clone(),
                E_::mk_id(ast_defs::Id(pos.clone(), "getAttribute".into())),
            ),
            nullflavor.clone(),
            false,
        ),
    );
    let args = vec![E(pos.clone(), E_::mk_string(string_utils::clean(s).into()))];
    emit_call(e, env, pos, &f, &[], &args[..], None, None)
}

fn emit_array_get(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    mode: Option<MemberOpMode>,
    query_op: QueryOp,
    base: &tast::Expr,
    elem: Option<&tast::Expr>,
    no_final: bool,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq, Option<usize>)> {
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

fn emit_array_get_(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    mode: Option<MemberOpMode>,
    query_op: QueryOp,
    base_expr: &tast::Expr,
    elem: Option<&tast::Expr>,
    no_final: bool,
    null_coalesce_assignment: bool,
    inout_param_info: Option<(usize, &inout_locals::AliasInfoMap)>,
) -> Result<(ArrayGetInstr, Option<usize>)> {
    use tast::Expr as E;
    match (base_expr, elem) {
        (E(pos, _), None) if !env.flags.contains(env::Flags::ALLOWS_ARRAY_APPEND) => Err(
            emit_fatal::raise_fatal_runtime(pos, "Can't use [] for reading"),
        ),
        _ => {
            let local_temp_kind = get_local_temp_kind(env, false, inout_param_info, elem);
            let mode = if null_coalesce_assignment {
                MemberOpMode::Warn
            } else {
                mode.unwrap_or(get_querym_op_mode(&query_op))
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
            )?;
            let cls_stack_size = match &base_result {
                ArrayGetBase::Regular(base) => base.cls_stack_size,
                ArrayGetBase::Inout { load, .. } => load.cls_stack_size,
            };
            let (memberkey, warninstr) =
                get_elem_member_key(e, env, cls_stack_size, elem, null_coalesce_assignment)?;
            let mut querym_n_unpopped = None;
            let mut make_final = |total_stack_size: StackIndex| -> InstrSeq {
                if no_final {
                    instr::empty()
                } else if null_coalesce_assignment {
                    querym_n_unpopped = Some(total_stack_size as usize);
                    instr::querym(0, query_op, memberkey.clone())
                } else {
                    instr::querym(total_stack_size as usize, query_op, memberkey.clone())
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
                        make_final(base.base_stack_size + base.cls_stack_size + elem_stack_size),
                    ]))
                }
                (ArrayGetBase::Regular(base), Some(local_kind)) => {
                    // base does not need temp locals but index expression does
                    let local = e.local_gen_mut().get_unnamed();
                    let load = vec![
                        // load base and indexer, value of indexer will be saved in local
                        (
                            InstrSeq::gather(vec![base.base_instrs.clone(), elem_instrs]),
                            Some((local.clone(), local_kind)),
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
                        )?,
                        instr::popc(),
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
                            make_final(base_stack_size + cls_stack_size + elem_stack_size),
                        ]),
                        None,
                    ));
                    let store =
                        InstrSeq::gather(vec![store, instr::setm(0, memberkey), instr::popc()]);
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
                    base_instrs.push((elem_instrs, Some((local.clone(), local_kind))));
                    base_instrs.push((
                        InstrSeq::gather(vec![
                            warninstr,
                            cls_instrs,
                            emit_pos(outer_pos),
                            setup_instrs,
                            make_final(base_stack_size + cls_stack_size + elem_stack_size),
                        ]),
                        None,
                    ));
                    let store = InstrSeq::gather(vec![
                        store,
                        instr::setm(0, MemberKey::EL(local, ReadOnlyOp::Any)),
                        instr::popc(),
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

fn is_special_class_constant_accessed_with_class_id(cname: &tast::ClassId_, id: &str) -> bool {
    let is_self_parent_or_static = match cname {
        tast::ClassId_::CIexpr(tast::Expr(_, tast::Expr_::Id(id))) => {
            string_utils::is_self(&id.1)
                || string_utils::is_parent(&id.1)
                || string_utils::is_static(&id.1)
        }
        _ => false,
    };
    string_utils::is_class(id) && !is_self_parent_or_static
}

fn emit_elem(
    e: &mut Emitter,
    env: &Env,
    elem: Option<&tast::Expr>,
    local_temp_kind: Option<StoredValueKind>,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq, StackIndex)> {
    Ok(match elem {
        None => (instr::empty(), 0),
        Some(expr) if expr.1.is_int() || expr.1.is_string() => (instr::empty(), 0),
        Some(expr) => match &expr.1 {
            tast::Expr_::Lvar(x) if !is_local_this(env, &x.1) => {
                if local_temp_kind.is_some() {
                    (
                        instr::cgetquietl(get_local(e, env, &x.0, local_id::get_name(&x.1))?),
                        0,
                    )
                } else if null_coalesce_assignment {
                    (
                        instr::cgetl(get_local(e, env, &x.0, local_id::get_name(&x.1))?),
                        1,
                    )
                } else {
                    (instr::empty(), 0)
                }
            }
            tast::Expr_::ClassConst(x)
                if is_special_class_constant_accessed_with_class_id(&(x.0).1, &(x.1).1) =>
            {
                (instr::empty(), 0)
            }
            _ => (emit_expr(e, env, expr)?, 1),
        },
    })
}

fn get_elem_member_key(
    e: &mut Emitter,
    env: &Env,
    stack_index: StackIndex,
    elem: Option<&tast::Expr>,
    null_coalesce_assignment: bool,
) -> Result<(MemberKey, InstrSeq)> {
    use tast::ClassId_ as CI_;
    use tast::Expr as E;
    use tast::Expr_ as E_;
    match elem {
        // ELement missing (so it's array append)
        None => Ok((MemberKey::W, instr::empty())),
        Some(elem_expr) => match &elem_expr.1 {
            // Special case for local
            E_::Lvar(x) if !is_local_this(env, &x.1) => Ok((
                {
                    if null_coalesce_assignment {
                        MemberKey::EC(stack_index, ReadOnlyOp::Any)
                    } else {
                        MemberKey::EL(
                            get_local(e, env, &x.0, local_id::get_name(&x.1))?,
                            ReadOnlyOp::Any,
                        )
                    }
                },
                instr::empty(),
            )),
            // Special case for literal integer
            E_::Int(s) => {
                match ast_constant_folder::expr_to_typed_value(e, &env.namespace, elem_expr) {
                    Ok(TypedValue::Int(i)) => {
                        Ok((MemberKey::EI(i, ReadOnlyOp::Any), instr::empty()))
                    }
                    _ => Err(Unrecoverable(format!("{} is not a valid integer index", s))),
                }
            }
            // Special case for literal string
            E_::String(s) => {
                Ok((
                    // FIXME: This is not safe--string literals are binary strings.
                    // There's no guarantee that they're valid UTF-8.
                    MemberKey::ET(
                        unsafe { String::from_utf8_unchecked(s.clone().into()) },
                        ReadOnlyOp::Any,
                    ),
                    instr::empty(),
                ))
            }
            // Special case for class name
            E_::ClassConst(x)
                if is_special_class_constant_accessed_with_class_id(&(x.0).1, &(x.1).1) =>
            {
                let cname =
                    match (&(x.0).1, env.scope.get_class()) {
                        (CI_::CIself, Some(cd)) => string_utils::strip_global_ns(cd.get_name_str()),
                        (CI_::CIexpr(E(_, E_::Id(id))), _) => string_utils::strip_global_ns(&id.1),
                        (CI_::CI(id), _) => string_utils::strip_global_ns(&id.1),
                        _ => return Err(Unrecoverable(
                            "Unreachable due to is_special_class_constant_accessed_with_class_id"
                                .into(),
                        )),
                    };
                let fq_id = class::Type::from_ast_name(&cname).to_raw_string().into();
                if e.options().emit_class_pointers() > 0 {
                    Ok((
                        MemberKey::ET(fq_id, ReadOnlyOp::Any),
                        instr::raise_class_string_conversion_warning(),
                    ))
                } else {
                    Ok((MemberKey::ET(fq_id, ReadOnlyOp::Any), instr::empty()))
                }
            }
            _ => {
                // General case
                Ok((MemberKey::EC(stack_index, ReadOnlyOp::Any), instr::empty()))
            }
        },
    }
}

fn emit_store_for_simple_base(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    elem_stack_size: isize,
    base: &tast::Expr,
    local: local::Type,
    is_base: bool,
) -> Result {
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
    )?;
    let memberkey = MemberKey::EL(local, ReadOnlyOp::Any);
    Ok(InstrSeq::gather(vec![
        base_expr_instrs_begin,
        base_expr_instrs_end,
        emit_pos(pos),
        base_setup_instrs,
        if is_base {
            instr::dim(MemberOpMode::Define, memberkey)
        } else {
            instr::setm(0, memberkey)
        },
    ]))
}

fn get_querym_op_mode(query_op: &QueryOp) -> MemberOpMode {
    match query_op {
        QueryOp::InOut => MemberOpMode::InOut,
        QueryOp::CGet => MemberOpMode::Warn,
        _ => MemberOpMode::ModeNone,
    }
}

fn emit_class_get(
    e: &mut Emitter,
    env: &Env,
    query_op: QueryOp,
    cid: &tast::ClassId,
    prop: &tast::ClassGetExpr,
) -> Result {
    let cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
    Ok(InstrSeq::gather(vec![
        InstrSeq::from(emit_class_expr(e, env, cexpr, prop)?),
        match query_op {
            QueryOp::CGet => instr::cgets(ReadOnlyOp::Any),
            QueryOp::Isset => instr::issets(),
            QueryOp::CGetQuiet => return Err(Unrecoverable("emit_class_get: CGetQuiet".into())),
            QueryOp::InOut => return Err(Unrecoverable("emit_class_get: InOut".into())),
        },
    ]))
}

fn emit_conditional_expr(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    etest: &tast::Expr,
    etrue: &Option<tast::Expr>,
    efalse: &tast::Expr,
) -> Result {
    Ok(match etrue.as_ref() {
        Some(etrue) => {
            let false_label = e.label_gen_mut().next_regular();
            let end_label = e.label_gen_mut().next_regular();
            let r = emit_jmpz(e, env, etest, &false_label)?;
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
                    instr::jmp(end_label.clone()),
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
                instr::jmpnz(end_label.clone()),
                instr::popc(),
                efalse_instr,
                instr::label(end_label),
            ])
        }
    })
}

fn emit_local(e: &mut Emitter, env: &Env, notice: BareThisOp, lid: &aast_defs::Lid) -> Result {
    let tast::Lid(pos, id) = lid;
    let id_name = local_id::get_name(id);
    if superglobals::is_superglobal(id_name) {
        Ok(InstrSeq::gather(vec![
            instr::string(string_utils::locals::strip_dollar(id_name)),
            emit_pos(pos),
            instr::cgetg(),
        ]))
    } else {
        let local = get_local(e, env, pos, id_name)?;
        Ok(
            if is_local_this(env, id) && !env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) {
                emit_pos_then(pos, instr::barethis(notice))
            } else {
                instr::cgetl(local)
            },
        )
    }
}

fn emit_class_const(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    cid: &tast::ClassId,
    id: &ast_defs::Pstring,
) -> Result {
    let mut cexpr = ClassExpr::class_id_to_class_expr(e, false, true, &env.scope, cid);
    if let ClassExpr::Id(ast_defs::Id(_, name)) = &cexpr {
        if let Some(reified_var_cexpr) = get_reified_var_cexpr(env, pos, &name)? {
            cexpr = reified_var_cexpr;
        }
    }
    match cexpr {
        ClassExpr::Id(ast_defs::Id(pos, name)) => {
            let cid = class::Type::from_ast_name_and_mangle(&name);
            let cname = cid.to_raw_string();
            Ok(if string_utils::is_class(&id.1) {
                if e.options().emit_class_pointers() == 1 {
                    emit_pos_then(&pos, instr::resolveclass(cid))
                } else if e.options().emit_class_pointers() == 2 {
                    emit_pos_then(&pos, instr::lazyclass(cid))
                } else {
                    emit_pos_then(&pos, instr::string(cname))
                }
            } else {
                emit_symbol_refs::add_class(e, cid.clone());
                // TODO(hrust) enabel `let const_id = r#const::Type::from_ast_name(&id.1);`,
                // `from_ast_name` should be able to accpet Cow<str>
                let const_id: r#const::Type =
                    string_utils::strip_global_ns(&id.1).to_string().into();
                emit_pos_then(&pos, instr::clscnsd(const_id, cid))
            })
        }
        _ => {
            let load_const = if string_utils::is_class(&id.1) {
                instr::classname()
            } else {
                // TODO(hrust) enabel `let const_id = r#const::Type::from_ast_name(&id.1);`,
                // `from_ast_name` should be able to accpet Cow<str>
                let const_id: r#const::Type =
                    string_utils::strip_global_ns(&id.1).to_string().into();
                instr::clscns(const_id)
            };
            if string_utils::is_class(&id.1) && e.options().emit_class_pointers() > 0 {
                emit_load_class_ref(e, env, pos, cexpr)
            } else {
                Ok(InstrSeq::gather(vec![
                    emit_load_class_ref(e, env, pos, cexpr)?,
                    load_const,
                ]))
            }
        }
    }
}

fn emit_unop(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    (uop, expr): &(ast_defs::Uop, tast::Expr),
) -> Result {
    use ast_defs::Uop as U;
    match uop {
        U::Utild | U::Unot => Ok(InstrSeq::gather(vec![
            emit_expr(e, env, expr)?,
            emit_pos_then(pos, from_unop(e.options(), uop)?),
        ])),
        U::Uplus | U::Uminus => Ok(InstrSeq::gather(vec![
            emit_pos(pos),
            instr::int(0),
            emit_expr(e, env, expr)?,
            emit_pos_then(pos, from_unop(e.options(), uop)?),
        ])),
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
            Ok(InstrSeq::gather(vec![
                emit_pos(pos),
                instr::silence_start(temp_local.clone()),
                {
                    let try_instrs = emit_expr(e, env, expr)?;
                    let catch_instrs = InstrSeq::gather(vec![
                        emit_pos(pos),
                        instr::silence_end(temp_local.clone()),
                    ]);
                    InstrSeq::create_try_catch(
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

fn from_unop(opts: &Options, op: &ast_defs::Uop) -> Result {
    use ast_defs::Uop as U;
    Ok(match op {
        U::Utild => instr::bitnot(),
        U::Unot => instr::not(),
        U::Uplus => {
            if opts.check_int_overflow() {
                instr::addo()
            } else {
                instr::add()
            }
        }
        U::Uminus => {
            if opts.check_int_overflow() {
                instr::subo()
            } else {
                instr::sub()
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

fn optimize_null_checks(e: &Emitter) -> bool {
    e.options()
        .hack_compiler_flags
        .contains(CompilerFlags::OPTIMIZE_NULL_CHECKS)
}

fn from_binop(opts: &Options, op: &ast_defs::Bop) -> Result {
    use ast_defs::Bop as B;
    Ok(match op {
        B::Plus => {
            if opts.check_int_overflow() {
                instr::addo()
            } else {
                instr::add()
            }
        }
        B::Minus => {
            if opts.check_int_overflow() {
                instr::subo()
            } else {
                instr::sub()
            }
        }
        B::Star => {
            if opts.check_int_overflow() {
                instr::mulo()
            } else {
                instr::mul()
            }
        }
        B::Slash => instr::div(),
        B::Eqeq => instr::eq(),
        B::Eqeqeq => instr::same(),
        B::Starstar => instr::pow(),
        B::Diff => instr::neq(),
        B::Diff2 => instr::nsame(),
        B::Lt => instr::lt(),
        B::Lte => instr::lte(),
        B::Gt => instr::gt(),
        B::Gte => instr::gte(),
        B::Dot => instr::concat(),
        B::Amp => instr::bitand(),
        B::Bar => instr::bitor(),
        B::Ltlt => instr::shl(),
        B::Gtgt => instr::shr(),
        B::Cmp => instr::cmp(),
        B::Percent => instr::mod_(),
        B::Xor => instr::bitxor(),
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

fn emit_first_expr(e: &mut Emitter, env: &Env, expr: &tast::Expr) -> Result<(InstrSeq, bool)> {
    Ok(match &expr.1 {
        tast::Expr_::Lvar(l)
            if !((is_local_this(env, &l.1) && !env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS))
                || superglobals::is_any_global(local_id::get_name(&l.1))) =>
        {
            (
                instr::cgetl2(get_local(e, env, &l.0, local_id::get_name(&l.1))?),
                true,
            )
        }
        _ => (emit_expr(e, env, expr)?, false),
    })
}

pub fn emit_two_exprs(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    e1: &tast::Expr,
    e2: &tast::Expr,
) -> Result {
    let (instrs1, is_under_top) = emit_first_expr(e, env, e1)?;
    let instrs2 = emit_expr(e, env, e2)?;
    let instrs2_is_var = e2.1.is_lvar();
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

fn emit_quiet_expr(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    expr: &tast::Expr,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq, Option<NumParams>)> {
    match &expr.1 {
        tast::Expr_::Lvar(lid) if !is_local_this(env, &lid.1) => Ok((
            instr::cgetquietl(get_local(e, env, pos, local_id::get_name(&lid.1))?),
            None,
        )),
        tast::Expr_::ArrayGet(x) => emit_array_get(
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
        tast::Expr_::ObjGet(x) => {
            if x.as_ref().3 {
                Ok((emit_expr(e, env, expr)?, None))
            } else {
                emit_obj_get(
                    e,
                    env,
                    pos,
                    QueryOp::CGetQuiet,
                    &x.0,
                    &x.1,
                    &x.2,
                    null_coalesce_assignment,
                )
            }
        }
        _ => Ok((emit_expr(e, env, expr)?, None)),
    }
}

fn emit_null_coalesce_assignment(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    e1: &tast::Expr,
    e2: &tast::Expr,
) -> Result {
    let end_label = e.label_gen_mut().next_regular();
    let do_set_label = e.label_gen_mut().next_regular();
    let l_nonnull = e.local_gen_mut().get_unnamed();
    let (quiet_instr, querym_n_unpopped) = emit_quiet_expr(e, env, pos, e1, true)?;
    let emit_popc_n = |n_unpopped| match n_unpopped {
        Some(n) => InstrSeq::gather(iter::repeat(instr::popc()).take(n).collect::<Vec<_>>()),
        None => instr::empty(),
    };
    Ok(InstrSeq::gather(vec![
        quiet_instr,
        instr::dup(),
        instr::istypec(IstypeOp::OpNull),
        instr::jmpnz(do_set_label.clone()),
        instr::popl(l_nonnull.clone()),
        emit_popc_n(querym_n_unpopped),
        instr::pushl(l_nonnull),
        instr::jmp(end_label.clone()),
        instr::label(do_set_label),
        instr::popc(),
        emit_lval_op(e, env, pos, LValOp::Set, e1, Some(e2), true)?,
        instr::label(end_label),
    ]))
}

fn emit_short_circuit_op(e: &mut Emitter, env: &Env, pos: &Pos, expr: &tast::Expr) -> Result {
    let its_true = e.label_gen_mut().next_regular();
    let its_done = e.label_gen_mut().next_regular();
    let jmp_instrs = emit_jmpnz(e, env, expr, &its_true)?;
    Ok(if jmp_instrs.is_fallthrough {
        InstrSeq::gather(vec![
            jmp_instrs.instrs,
            emit_pos(pos),
            instr::false_(),
            instr::jmp(its_done.clone()),
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

fn emit_binop(e: &mut Emitter, env: &Env, pos: &Pos, expr: &tast::Expr) -> Result {
    let (op, e1, e2) = expr.1.as_binop().unwrap();
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
            Ok(InstrSeq::gather(vec![
                emit_quiet_expr(e, env, pos, e1, false)?.0,
                instr::dup(),
                instr::istypec(IstypeOp::OpNull),
                instr::not(),
                instr::jmpnz(end_label.clone()),
                instr::popc(),
                rhs,
                instr::label(end_label),
            ]))
        }
        _ => {
            let default = |e: &mut Emitter| {
                Ok(InstrSeq::gather(vec![
                    emit_two_exprs(e, env, pos, e1, e2)?,
                    from_binop(e.options(), op)?,
                ]))
            };
            if optimize_null_checks(e) {
                match op {
                    B::Eqeqeq if e2.1.is_null() => emit_is_null(e, env, e1),
                    B::Eqeqeq if e1.1.is_null() => emit_is_null(e, env, e2),
                    B::Diff2 if e2.1.is_null() => Ok(InstrSeq::gather(vec![
                        emit_is_null(e, env, e1)?,
                        instr::not(),
                    ])),
                    B::Diff2 if e1.1.is_null() => Ok(InstrSeq::gather(vec![
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

fn emit_pipe(
    e: &mut Emitter,
    env: &Env,
    (_, e1, e2): &(aast_defs::Lid, tast::Expr, tast::Expr),
) -> Result {
    let lhs_instrs = emit_expr(e, env, e1)?;
    scope::with_unnamed_local(e, |e, local| {
        // TODO(hrust) avoid cloning env
        let mut pipe_env = env.clone();
        pipe_env.with_pipe_var(local.clone());
        let rhs_instrs = emit_expr(e, &pipe_env, e2)?;
        Ok((
            InstrSeq::gather(vec![lhs_instrs, instr::popl(local.clone())]),
            rhs_instrs,
            instr::unsetl(local),
        ))
    })
}

fn emit_as(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    (expr, h, is_nullable): &(tast::Expr, aast_defs::Hint, bool),
) -> Result {
    e.local_scope(|e| {
        let arg_local = e.local_gen_mut().get_unnamed();
        let type_struct_local = e.local_gen_mut().get_unnamed();
        let (ts_instrs, is_static) = emit_reified_arg(e, env, pos, true, h)?;
        let then_label = e.label_gen_mut().next_regular();
        let done_label = e.label_gen_mut().next_regular();
        let main_block = |ts_instrs, resolve| {
            InstrSeq::gather(vec![
                ts_instrs,
                instr::setl(type_struct_local.clone()),
                match resolve {
                    TypestructResolveOp::Resolve => instr::is_type_structc_resolve(),
                    TypestructResolveOp::DontResolve => instr::is_type_structc_dontresolve(),
                },
                instr::jmpnz(then_label.clone()),
                if *is_nullable {
                    InstrSeq::gather(vec![instr::null(), instr::jmp(done_label.clone())])
                } else {
                    InstrSeq::gather(vec![
                        instr::pushl(arg_local.clone()),
                        instr::pushl(type_struct_local.clone()),
                        instr::throwastypestructexception(),
                    ])
                },
            ])
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
        Ok(InstrSeq::gather(vec![
            i1,
            instr::setl(arg_local.clone()),
            i2,
            instr::label(then_label),
            instr::pushl(arg_local),
            instr::unsetl(type_struct_local),
            instr::label(done_label),
        ]))
    })
}

fn emit_cast(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    hint: &aast_defs::Hint_,
    expr: &tast::Expr,
) -> Result {
    use aast_defs::Hint_ as H_;
    let op = match hint {
        H_::Happly(ast_defs::Id(_, id), hints) if hints.is_empty() => {
            let id = string_utils::strip_ns(id);
            match string_utils::strip_hh_ns(&id).as_ref() {
                typehints::INT => instr::cast_int(),
                typehints::BOOL => instr::cast_bool(),
                typehints::STRING => instr::cast_string(),
                typehints::FLOAT => instr::cast_double(),
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
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        emit_pos(pos),
        op,
    ]))
}

pub fn emit_unset_expr(e: &mut Emitter, env: &Env, expr: &tast::Expr) -> Result {
    emit_lval_op_nonlist(
        e,
        env,
        &expr.0,
        LValOp::Unset,
        expr,
        instr::empty(),
        0,
        false,
    )
}

pub fn emit_set_range_expr(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    name: &str,
    kind: Setrange,
    args: &[&tast::Expr],
) -> Result {
    let raise_fatal = |msg: &str| {
        Err(emit_fatal::raise_fatal_parse(
            pos,
            format!("{} {}", name, msg),
        ))
    };

    let (base, offset, src, args) = if args.len() >= 3 {
        (&args[0], &args[1], &args[2], &args[3..])
    } else {
        return raise_fatal("expects at least 3 arguments");
    };

    let count_instrs = match (args, kind.vec) {
        ([c], true) => emit_expr(e, env, c)?,
        ([], _) => instr::int(-1),
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
        false, /*null_coalesce_assignment*/
        3,     /* base_offset */
        3,     /* rhs_stack_size */
    )?;
    Ok(InstrSeq::gather(vec![
        base_expr,
        cls_expr,
        emit_expr(e, env, offset)?,
        emit_expr(e, env, src)?,
        count_instrs,
        base_setup,
        instr::instr(Instruct::IFinal(InstructFinal::SetRangeM(
            (base_stack + cls_stack)
                .try_into()
                .expect("StackIndex overflow"),
            kind.size.try_into().expect("Setrange size overflow"),
            kind.op,
            ReadOnlyOp::Any,
        ))),
    ]))
}

pub fn is_reified_tparam(env: &Env, is_fun: bool, name: &str) -> Option<(usize, bool)> {
    let is = |tparams: &[tast::Tparam]| {
        let is_soft = |ual: &Vec<tast::UserAttribute>| {
            ual.iter().any(|ua| user_attributes::is_soft(&ua.name.1))
        };
        use tast::ReifyKind::*;
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
///   2. base selector instructions: a sequence of Base/Dim instructions that
///      actually constructs the base address from "member keys" that are inlined
///      in the instructions, or pulled from the key values that
///      were pushed on the stack in section 1.
///   3. (constructed by the caller) a final accessor e.g. QueryM or setter
///      e.g. SetOpM instruction that has the final key inlined in the
///      instruction, or pulled from the key values that were pushed on the
///      stack in section 1.
/// The function returns a triple (base_instrs, base_setup_instrs, stack_size)
/// where base_instrs is section 1 above, base_setup_instrs is section 2, and
/// stack_size is the number of values pushed onto the stack by section 1.
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
///)
fn emit_base(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    mode: MemberOpMode,
    is_object: bool,
    notice: BareThisOp,
    null_coalesce_assignment: bool,
    base_offset: StackIndex,
    rhs_stack_size: StackIndex,
) -> Result<(InstrSeq, InstrSeq, InstrSeq, StackIndex, StackIndex)> {
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

fn is_trivial(env: &Env, is_base: bool, expr: &tast::Expr) -> bool {
    use tast::Expr_ as E_;
    match &expr.1 {
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

fn get_local_temp_kind(
    env: &Env,
    is_base: bool,
    inout_param_info: Option<(usize, &inout_locals::AliasInfoMap)>,
    expr: Option<&tast::Expr>,
) -> Option<StoredValueKind> {
    match (expr, inout_param_info) {
        (_, None) => None,
        (Some(tast::Expr(_, tast::Expr_::Lvar(id))), Some((i, aliases)))
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

fn emit_base_(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    mode: MemberOpMode,
    is_object: bool,
    notice: BareThisOp,
    null_coalesce_assignment: bool,
    base_offset: StackIndex,
    rhs_stack_size: StackIndex,
    inout_param_info: Option<(usize, &inout_locals::AliasInfoMap)>,
) -> Result<ArrayGetBase> {
    let pos = &expr.0;
    let expr_ = &expr.1;
    let base_mode = if mode == MemberOpMode::InOut {
        MemberOpMode::Warn
    } else {
        mode
    };
    let local_temp_kind = get_local_temp_kind(env, true, inout_param_info, Some(expr));
    let emit_default = |
        e: &mut Emitter,
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
                        base_instrs: vec![(base_instrs, Some((local.clone(), local_temp)))],
                        cls_instrs,
                        setup_instrs,
                        base_stack_size,
                        cls_stack_size,
                    },
                    store: instr::basel(local, MemberOpMode::Define),
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

    let emit_expr_default = |e: &mut Emitter, env, expr: &tast::Expr| -> Result<ArrayGetBase> {
        let base_expr_instrs = emit_expr(e, env, expr)?;
        Ok(emit_default(
            e,
            base_expr_instrs,
            instr::empty(),
            emit_pos_then(pos, instr::basec(base_offset, base_mode)),
            1,
            0,
        ))
    };

    use tast::Expr_ as E_;
    match expr_ {
        E_::Lvar(x) if superglobals::is_superglobal(&(x.1).1) => {
            let base_instrs = emit_pos_then(
                &x.0,
                instr::string(string_utils::locals::strip_dollar(&(x.1).1)),
            );

            Ok(emit_default(
                e,
                base_instrs,
                instr::empty(),
                instr::basegc(base_offset, base_mode),
                1,
                0,
            ))
        }
        E_::Lvar(x) if is_object && &(x.1).1 == special_idents::THIS => {
            let base_instrs = emit_pos_then(&x.0, instr::checkthis());
            Ok(emit_default(
                e,
                base_instrs,
                instr::empty(),
                instr::baseh(),
                0,
                0,
            ))
        }
        E_::Lvar(x)
            if !is_local_this(env, &x.1) || env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) =>
        {
            let v = get_local(e, env, &x.0, &(x.1).1)?;
            let base_instr = if local_temp_kind.is_some() {
                instr::cgetquietl(v.clone())
            } else {
                instr::empty()
            };
            Ok(emit_default(
                e,
                base_instr,
                instr::empty(),
                instr::basel(v, base_mode),
                0,
                0,
            ))
        }
        E_::Lvar(lid) => {
            let local = emit_local(e, env, notice, lid)?;
            Ok(emit_default(
                e,
                local,
                instr::empty(),
                instr::basec(base_offset, base_mode),
                1,
                0,
            ))
        }
        E_::ArrayGet(x) => match (&(x.0).1, x.1.as_ref()) {
            // $a[] can not be used as the base of an array get unless as an lval
            (_, None) if !env.flags.contains(env::Flags::ALLOWS_ARRAY_APPEND) => {
                return Err(emit_fatal::raise_fatal_runtime(
                    pos,
                    "Can't use [] for reading",
                ));
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
                let make_setup_instrs = |base_setup_instrs: InstrSeq| {
                    InstrSeq::gather(vec![
                        warninstr,
                        base_setup_instrs,
                        instr::dim(mode, mk.clone()),
                    ])
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
                                base_instrs: vec![(base_instrs, Some((local.clone(), local_temp)))],
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
                            store: InstrSeq::gather(vec![
                                store,
                                instr::dim(MemberOpMode::Define, mk),
                            ]),
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
                        base_instrs.push((elem_instrs, Some((local.clone(), local_kind))));
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
                                instr::dim(
                                    MemberOpMode::Define,
                                    MemberKey::EL(local, ReadOnlyOp::Any),
                                ),
                            ]),
                        }
                    }
                })
            }
        },
        E_::ObjGet(x) => {
            if x.as_ref().3 {
                emit_expr_default(e, env, expr)
            } else {
                let (base_expr, prop_expr, null_flavor, _) = &**x;
                Ok(match prop_expr.1.as_id() {
                    Some(ast_defs::Id(_, s)) if string_utils::is_xhp(&s) => {
                        let base_instrs =
                            emit_xhp_obj_get(e, env, pos, base_expr, &s, null_flavor)?;
                        emit_default(
                            e,
                            base_instrs,
                            instr::empty(),
                            instr::basec(base_offset, base_mode),
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
                        )?;
                        let (mk, prop_instrs, _) = emit_prop_expr(
                            e,
                            env,
                            null_flavor,
                            base_offset + cls_stack_size,
                            prop_expr,
                            null_coalesce_assignment,
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
        }
        E_::ClassGet(x) => {
            if x.2 {
                emit_expr_default(e, env, expr)
            } else {
                let (cid, prop, _) = &**x;
                let cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
                let (cexpr_begin, cexpr_end) = emit_class_expr(e, env, cexpr, prop)?;
                Ok(emit_default(
                    e,
                    cexpr_begin,
                    cexpr_end,
                    instr::basesc(base_offset + 1, rhs_stack_size, base_mode, ReadOnlyOp::Any),
                    1,
                    1,
                ))
            }
        }
        _ => emit_expr_default(e, env, expr),
    }
}

pub fn emit_ignored_exprs(
    emitter: &mut Emitter,
    env: &Env,
    pos: &Pos,
    exprs: &[tast::Expr],
) -> Result {
    exprs
        .iter()
        .map(|e| emit_ignored_expr(emitter, env, pos, e))
        .collect::<Result<Vec<_>>>()
        .map(InstrSeq::gather)
}

// TODO(hrust): change pos from &Pos to Option<&Pos>, since Pos::make_none() still allocate mem.
pub fn emit_ignored_expr(emitter: &mut Emitter, env: &Env, pos: &Pos, expr: &tast::Expr) -> Result {
    Ok(InstrSeq::gather(vec![
        emit_expr(emitter, env, expr)?,
        emit_pos_then(pos, instr::popc()),
    ]))
}

pub fn emit_lval_op(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    op: LValOp,
    expr1: &tast::Expr,
    expr2: Option<&tast::Expr>,
    null_coalesce_assignment: bool,
) -> Result {
    match (op, &expr1.1, expr2) {
        (LValOp::Set, tast::Expr_::List(l), Some(expr2)) => {
            let instr_rhs = emit_expr(e, env, expr2)?;
            let has_elements = l.iter().any(|e| !e.1.is_omitted());
            if !has_elements {
                Ok(instr_rhs)
            } else {
                scope::with_unnamed_local(e, |e, local| {
                    let loc = if can_use_as_rhs_in_list_assignment(&expr2.1)? {
                        Some(&local)
                    } else {
                        None
                    };
                    let (instr_lhs, instr_assign) =
                        emit_lval_op_list(e, env, pos, loc, &[], expr1, false)?;
                    Ok((
                        InstrSeq::gather(vec![instr_lhs, instr_rhs, instr::popl(local.clone())]),
                        instr_assign,
                        instr::pushl(local),
                    ))
                })
            }
        }
        _ => e.local_scope(|e| {
            let (rhs_instrs, rhs_stack_size) = match expr2 {
                None => (instr::empty(), 0),
                Some(tast::Expr(_, tast::Expr_::Yield(af))) => {
                    let temp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            emit_yield(e, env, pos, af)?,
                            instr::setl(temp.clone()),
                            instr::popc(),
                            instr::pushl(temp),
                        ]),
                        1,
                    )
                }
                Some(expr) => (emit_expr(e, env, expr)?, 1),
            };
            emit_lval_op_nonlist(
                e,
                env,
                pos,
                op,
                expr1,
                rhs_instrs,
                rhs_stack_size,
                null_coalesce_assignment,
            )
        }),
    }
}

fn can_use_as_rhs_in_list_assignment(expr: &tast::Expr_) -> Result<bool> {
    use aast::Expr_ as E_;
    Ok(match expr {
        E_::Call(c)
            if ((c.0).1)
                .as_id()
                .map_or(false, |id| id.1 == special_functions::ECHO) =>
        {
            false
        }
        E_::ObjGet(o) if !o.as_ref().3 => true,
        E_::ClassGet(c) if !c.as_ref().2 => true,
        E_::Lvar(_)
        | E_::ArrayGet(_)
        | E_::Call(_)
        | E_::FunctionPointer(_)
        | E_::New(_)
        | E_::Record(_)
        | E_::Yield(_)
        | E_::Cast(_)
        | E_::Eif(_)
        | E_::Varray(_)
        | E_::Darray(_)
        | E_::Collection(_)
        | E_::Clone(_)
        | E_::Unop(_)
        | E_::As(_)
        | E_::Await(_)
        | E_::ReadonlyExpr(_)
        | E_::ClassConst(_) => true,
        E_::Pipe(p) => can_use_as_rhs_in_list_assignment(&(p.2).1)?,
        E_::Binop(b) => {
            if let ast_defs::Bop::Eq(None) = &b.0 {
                if (b.1).1.is_list() {
                    return can_use_as_rhs_in_list_assignment(&(b.2).1);
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
fn emit_array_get_fixed(last_usage: bool, local: local::Type, indices: &[isize]) -> InstrSeq {
    let (base, stack_count) = if last_usage {
        (
            InstrSeq::gather(vec![
                instr::pushl(local),
                instr::basec(0, MemberOpMode::Warn),
            ]),
            1,
        )
    } else {
        (instr::basel(local, MemberOpMode::Warn), 0)
    };
    let indices = InstrSeq::gather(
        indices
            .iter()
            .enumerate()
            .rev()
            .map(|(i, ix)| {
                let mk = MemberKey::EI(*ix as i64, ReadOnlyOp::Any);
                if i == 0 {
                    instr::querym(stack_count, QueryOp::CGet, mk)
                } else {
                    instr::dim(MemberOpMode::Warn, mk)
                }
            })
            .collect(),
    );
    InstrSeq::gather(vec![base, indices])
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
pub fn emit_lval_op_list(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    local: Option<&local::Type>,
    indices: &[isize],
    expr: &tast::Expr,
    last_usage: bool,
) -> Result<(InstrSeq, InstrSeq)> {
    use options::Php7Flags;
    use tast::Expr_ as E_;
    let is_ltr = e.options().php7_flags.contains(Php7Flags::LTR_ASSIGN);
    match &expr.1 {
        E_::List(exprs) => {
            let last_non_omitted = if last_usage {
                // last usage of the local will happen when processing last non-omitted
                // element in the list - find it
                if is_ltr {
                    exprs.iter().rposition(|v| !v.1.is_omitted())
                } else {
                    // in right-to-left case result list will be reversed
                    // so we need to find first non-omitted expression
                    exprs.iter().rev().rposition(|v| !v.1.is_omitted())
                }
            } else {
                None
            };
            let (lhs_instrs, set_instrs): (Vec<InstrSeq>, Vec<InstrSeq>) = exprs
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
        E_::Omitted => Ok((instr::empty(), instr::empty())),
        _ => {
            // Generate code to access the element from the array
            let access_instrs = match (local, indices) {
                (Some(loc), [_, ..]) => emit_array_get_fixed(last_usage, loc.to_owned(), indices),
                (Some(loc), []) => {
                    if last_usage {
                        instr::pushl(loc.to_owned())
                    } else {
                        instr::cgetl(loc.to_owned())
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
                false,
            )?;
            Ok(if is_ltr {
                (
                    instr::empty(),
                    InstrSeq::gather(vec![lhs_instrs, rhs_instrs, set_op, instr::popc()]),
                )
            } else {
                (
                    lhs_instrs,
                    InstrSeq::gather(vec![instr::empty(), rhs_instrs, set_op, instr::popc()]),
                )
            })
        }
    }
}

pub fn emit_lval_op_nonlist(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    op: LValOp,
    expr: &tast::Expr,
    rhs_instrs: InstrSeq,
    rhs_stack_size: isize,
    null_coalesce_assignment: bool,
) -> Result {
    emit_lval_op_nonlist_steps(
        e,
        env,
        outer_pos,
        op,
        expr,
        rhs_instrs,
        rhs_stack_size,
        null_coalesce_assignment,
    )
    .map(|(lhs, rhs, setop)| InstrSeq::gather(vec![lhs, rhs, setop]))
}

pub fn emit_final_global_op(pos: &Pos, op: LValOp) -> InstrSeq {
    use LValOp as L;
    match op {
        L::Set => emit_pos_then(pos, instr::setg()),
        L::SetOp(op) => instr::setopg(op),
        L::IncDec(op) => instr::incdecg(op),
        L::Unset => emit_pos_then(pos, instr::unsetg()),
    }
}

pub fn emit_final_local_op(pos: &Pos, op: LValOp, lid: local::Type) -> InstrSeq {
    use LValOp as L;
    emit_pos_then(
        pos,
        match op {
            L::Set => instr::setl(lid),
            L::SetOp(op) => instr::setopl(lid, op),
            L::IncDec(op) => instr::incdecl(lid, op),
            L::Unset => instr::unsetl(lid),
        },
    )
}

fn emit_final_member_op(stack_size: usize, op: LValOp, mk: MemberKey) -> InstrSeq {
    use LValOp as L;
    match op {
        L::Set => instr::setm(stack_size, mk),
        L::SetOp(op) => instr::setopm(stack_size, op, mk),
        L::IncDec(op) => instr::incdecm(stack_size, op, mk),
        L::Unset => instr::unsetm(stack_size, mk),
    }
}

fn emit_final_static_op(cid: &tast::ClassId, prop: &tast::ClassGetExpr, op: LValOp) -> Result {
    use LValOp as L;
    Ok(match op {
        L::Set => instr::sets(ReadOnlyOp::Any),
        L::SetOp(op) => instr::setops(op, ReadOnlyOp::Any),
        L::IncDec(op) => instr::incdecs(op, ReadOnlyOp::Any),
        L::Unset => {
            let pos = match prop {
                tast::ClassGetExpr::CGstring((pos, _))
                | tast::ClassGetExpr::CGexpr(tast::Expr(pos, _)) => pos,
            };
            let cid = text_of_class_id(cid);
            let id = text_of_prop(prop);
            emit_fatal::emit_fatal_runtime(
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

pub fn emit_lval_op_nonlist_steps(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    op: LValOp,
    expr: &tast::Expr,
    rhs_instrs: InstrSeq,
    rhs_stack_size: isize,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq, InstrSeq, InstrSeq)> {
    let f = |env: &mut Env| {
        use tast::Expr_ as E_;
        let pos = &expr.0;
        Ok(match &expr.1 {
            E_::Lvar(v) if superglobals::is_any_global(local_id::get_name(&v.1)) => (
                emit_pos_then(
                    &v.0,
                    instr::string(string_utils::lstrip(local_id::get_name(&v.1), "$")),
                ),
                rhs_instrs,
                emit_final_global_op(outer_pos, op),
            ),
            E_::Lvar(v) if is_local_this(env, &v.1) && op.is_incdec() => (
                emit_local(e, env, BareThisOp::Notice, v)?,
                rhs_instrs,
                instr::empty(),
            ),
            E_::Lvar(v) if !is_local_this(env, &v.1) || op == LValOp::Unset => {
                (instr::empty(), rhs_instrs, {
                    let lid = get_local(e, env, &v.0, &(v.1).1)?;
                    emit_final_local_op(outer_pos, op, lid)
                })
            }
            E_::ArrayGet(x) => match (&(x.0).1, x.1.as_ref()) {
                (_, None) if !env.flags.contains(env::Flags::ALLOWS_ARRAY_APPEND) => {
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
                    let (mut elem_instrs, elem_stack_size) =
                        emit_elem(e, env, opt_elem_expr, None, null_coalesce_assignment)?;
                    if null_coalesce_assignment {
                        elem_instrs = instr::empty();
                    }
                    let base_offset = elem_stack_size + rhs_stack_size;
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
                        emit_pos_then(pos, emit_final_member_op(total_stack_size as usize, op, mk));
                    (
                        if null_coalesce_assignment {
                            elem_instrs
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
            E_::ObjGet(x) if !x.as_ref().3 => {
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
                let prop_stack_size =
                    emit_prop_expr(e, env, nullflavor, 0, e2, null_coalesce_assignment)?.2;
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
                )?;
                let (mk, mut prop_instrs, _) = emit_prop_expr(
                    e,
                    env,
                    nullflavor,
                    rhs_stack_size + cls_stack_size,
                    e2,
                    null_coalesce_assignment,
                )?;
                if null_coalesce_assignment {
                    prop_instrs = instr::empty();
                }
                let total_stack_size = prop_stack_size + base_stack_size + cls_stack_size;
                let final_instr =
                    emit_pos_then(pos, emit_final_member_op(total_stack_size as usize, op, mk));
                (
                    if null_coalesce_assignment {
                        prop_instrs
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
            E_::ClassGet(x) if !x.as_ref().2 => {
                let (cid, prop, _) = &**x;
                let cexpr = ClassExpr::class_id_to_class_expr(e, false, false, &env.scope, cid);
                let final_instr_ = emit_final_static_op(cid, prop, op)?;
                let final_instr = emit_pos_then(pos, final_instr_);
                (
                    InstrSeq::from(emit_class_expr(e, env, cexpr, prop)?),
                    rhs_instrs,
                    final_instr,
                )
            }
            E_::Unop(uop) => (
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
                    )?,
                    from_unop(e.options(), &uop.0)?,
                ]),
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
    let mut env = env.clone();
    match op {
        LValOp::Set | LValOp::SetOp(_) | LValOp::IncDec(_) => env.with_allows_array_append(f),
        _ => f(&mut env),
    }
}

fn emit_class_expr(
    e: &mut Emitter,
    env: &Env,
    cexpr: ClassExpr,
    prop: &tast::ClassGetExpr,
) -> Result<(InstrSeq, InstrSeq)> {
    let load_prop = |e: &mut Emitter| match prop {
        tast::ClassGetExpr::CGstring((pos, id)) => Ok(emit_pos_then(
            pos,
            instr::string(string_utils::locals::strip_dollar(id)),
        )),
        tast::ClassGetExpr::CGexpr(expr) => emit_expr(e, env, expr),
    };
    Ok(match &cexpr {
        ClassExpr::Expr(expr)
            if expr.1.is_call()
                || expr.1.is_binop()
                || expr.1.is_class_get()
                || expr
                    .1
                    .as_lvar()
                    .map_or(false, |tast::Lid(_, id)| local_id::get_name(id) == "$this") =>
        {
            let cexpr_local = emit_expr(e, env, expr)?;
            (
                instr::empty(),
                InstrSeq::gather(vec![
                    cexpr_local,
                    scope::stash_top_in_unnamed_local(e, load_prop)?,
                    instr::classgetc(),
                ]),
            )
        }
        _ => {
            let pos = match prop {
                tast::ClassGetExpr::CGstring((pos, _))
                | tast::ClassGetExpr::CGexpr(tast::Expr(pos, _)) => pos,
            };
            (load_prop(e)?, emit_load_class_ref(e, env, pos, cexpr)?)
        }
    })
}

pub fn fixup_type_arg<'a>(
    env: &Env,
    isas: bool,
    hint: &'a tast::Hint,
) -> Result<impl AsRef<tast::Hint> + 'a> {
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
            hf: &tast::HintFun,
        ) -> StdResult<(), Option<Error>> {
            hf.param_tys.accept(c, self.object())?;
            hf.return_ty.accept(c, self.object())
        }

        fn visit_hint(&mut self, c: &mut (), h: &tast::Hint) -> StdResult<(), Option<Error>> {
            use tast::{Hint_ as H_, Id};
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

        fn visit_hint_(&mut self, c: &mut (), h: &tast::Hint_) -> StdResult<(), Option<Error>> {
            use tast::{Hint_ as H_, Id};
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

        fn visit_hint_fun(&mut self, c: &mut (), hf: &mut tast::HintFun) -> StdResult<(), ()> {
            <Vec<tast::Hint> as NodeMut<Self::P>>::accept(&mut hf.param_tys, c, self.object())?;
            <tast::Hint as NodeMut<Self::P>>::accept(&mut hf.return_ty, c, self.object())
        }

        fn visit_hint_(&mut self, c: &mut (), h: &mut tast::Hint_) -> StdResult<(), ()> {
            use tast::{Hint_ as H_, Id};
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

pub fn emit_reified_arg(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    isas: bool,
    hint: &tast::Hint,
) -> Result<(InstrSeq, bool)> {
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

        fn visit_hint_(&mut self, c: &mut (), h_: &'ast tast::Hint_) -> StdResult<(), ()> {
            use tast::{Hint_ as H_, Id};
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
    fn f<'a>(mut acc: HashSet<&'a str>, tparam: &'a tast::Tparam) -> HashSet<&'a str> {
        if tparam.reified != tast::ReifyKind::Erased {
            acc.insert(&tparam.name.1);
        }
        acc
    }
    let current_tags = env
        .scope
        .get_fun_tparams()
        .iter()
        .fold(HashSet::<&str>::new(), |acc, t| f(acc, &*t));
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
        tast::Hint_::Happly(tast::Id(_, name), hs)
            if hs.is_empty() && current_tags.contains(name.as_str()) =>
        {
            Ok((emit_reified_type(env, pos, name)?, false))
        }
        _ => {
            let ts = get_type_structure_for_hint(e, &[], &collector.acc, hint)?;
            let ts_list = if collector.acc.is_empty() {
                ts
            } else {
                let values = collector
                    .acc
                    .iter()
                    .map(|v| emit_reified_type(env, pos, v))
                    .collect::<Result<Vec<_>>>()?;
                InstrSeq::gather(vec![InstrSeq::gather(values), ts])
            };
            Ok((
                InstrSeq::gather(vec![
                    ts_list,
                    instr::combine_and_resolve_type_struct((collector.acc.len() + 1) as isize),
                ]),
                collector.acc.is_empty(),
            ))
        }
    }
}

pub fn get_local(e: &mut Emitter, env: &Env, pos: &Pos, s: &str) -> Result<local::Type> {
    if s == special_idents::DOLLAR_DOLLAR {
        match &env.pipe_var {
            None => Err(emit_fatal::raise_fatal_runtime(
                pos,
                "Pipe variables must occur only in the RHS of pipe expressions",
            )),
            Some(var) => Ok(var.clone()),
        }
    } else if special_idents::is_tmp_var(s) {
        Ok(e.local_gen().get_unnamed_for_tempname(s).clone())
    } else {
        Ok(local::Type::Named(s.into()))
    }
}

pub fn emit_is_null(e: &mut Emitter, env: &Env, expr: &tast::Expr) -> Result {
    if let Some(tast::Lid(pos, id)) = expr.1.as_lvar() {
        if !is_local_this(env, id) {
            return Ok(instr::istypel(
                get_local(e, env, pos, local_id::get_name(id))?,
                IstypeOp::OpNull,
            ));
        }
    }

    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        instr::istypec(IstypeOp::OpNull),
    ]))
}

pub fn emit_jmpnz(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    label: &Label,
) -> Result<EmitJmpResult> {
    let tast::Expr(pos, expr_) = expr;
    let opt = optimize_null_checks(e);
    Ok(
        match ast_constant_folder::expr_to_typed_value(e, &env.namespace, expr) {
            Ok(tv) => {
                if Into::<bool>::into(tv) {
                    EmitJmpResult {
                        instrs: emit_pos_then(pos, instr::jmp(label.clone())),
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
                use {ast_defs::Uop as U, tast::Expr_ as E};
                match expr_ {
                    E::Unop(uo) if uo.0 == U::Unot => emit_jmpz(e, env, &uo.1, label)?,
                    E::Binop(bo) if bo.0.is_barbar() => {
                        let r1 = emit_jmpnz(e, env, &bo.1, label)?;
                        if r1.is_fallthrough {
                            let r2 = emit_jmpnz(e, env, &bo.2, label)?;
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
                    E::Binop(bo) if bo.0.is_ampamp() => {
                        let skip_label = e.label_gen_mut().next_regular();
                        let r1 = emit_jmpz(e, env, &bo.1, &skip_label)?;
                        if !r1.is_fallthrough {
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(vec![
                                        r1.instrs,
                                        InstrSeq::optional(
                                            r1.is_label_used,
                                            vec![instr::label(skip_label)],
                                        ),
                                    ]),
                                ),
                                is_fallthrough: r1.is_label_used,
                                is_label_used: false,
                            }
                        } else {
                            let r2 = emit_jmpnz(e, env, &bo.2, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(vec![
                                        r1.instrs,
                                        r2.instrs,
                                        InstrSeq::optional(
                                            r1.is_label_used,
                                            vec![instr::label(skip_label)],
                                        ),
                                    ]),
                                ),
                                is_fallthrough: r2.is_fallthrough || r1.is_label_used,
                                is_label_used: r2.is_label_used,
                            }
                        }
                    }
                    E::Binop(bo)
                        if bo.0.is_eqeqeq()
                            && ((bo.1).1.is_null() || (bo.2).1.is_null())
                            && opt =>
                    {
                        let is_null =
                            emit_is_null(e, env, if (bo.1).1.is_null() { &bo.2 } else { &bo.1 })?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![is_null, instr::jmpnz(label.clone())]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    E::Binop(bo)
                        if bo.0.is_diff2() && ((bo.1).1.is_null() || (bo.2).1.is_null()) && opt =>
                    {
                        let is_null =
                            emit_is_null(e, env, if (bo.1).1.is_null() { &bo.2 } else { &bo.1 })?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![is_null, instr::jmpz(label.clone())]),
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
                                InstrSeq::gather(vec![instr, instr::jmpnz(label.clone())]),
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

pub fn emit_jmpz(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    label: &Label,
) -> Result<EmitJmpResult> {
    let tast::Expr(pos, expr_) = expr;
    let opt = optimize_null_checks(e);
    Ok(
        match ast_constant_folder::expr_to_typed_value(e, &env.namespace, expr) {
            Ok(v) => {
                let b: bool = v.into();
                if b {
                    EmitJmpResult {
                        instrs: emit_pos_then(pos, instr::empty()),
                        is_fallthrough: true,
                        is_label_used: false,
                    }
                } else {
                    EmitJmpResult {
                        instrs: emit_pos_then(pos, instr::jmp(label.clone())),
                        is_fallthrough: false,
                        is_label_used: true,
                    }
                }
            }
            Err(_) => {
                use {ast_defs::Uop as U, tast::Expr_ as E};
                match expr_ {
                    E::Unop(uo) if uo.0 == U::Unot => emit_jmpnz(e, env, &uo.1, label)?,
                    E::Binop(bo) if bo.0.is_barbar() => {
                        let skip_label = e.label_gen_mut().next_regular();
                        let r1 = emit_jmpnz(e, env, &bo.1, &skip_label)?;
                        if !r1.is_fallthrough {
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(vec![
                                        r1.instrs,
                                        InstrSeq::optional(
                                            r1.is_label_used,
                                            vec![instr::label(skip_label)],
                                        ),
                                    ]),
                                ),
                                is_fallthrough: r1.is_label_used,
                                is_label_used: false,
                            }
                        } else {
                            let r2 = emit_jmpz(e, env, &bo.2, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    pos,
                                    InstrSeq::gather(vec![
                                        r1.instrs,
                                        r2.instrs,
                                        InstrSeq::optional(
                                            r1.is_label_used,
                                            vec![instr::label(skip_label)],
                                        ),
                                    ]),
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
                    E::Binop(bo)
                        if bo.0.is_eqeqeq()
                            && ((bo.1).1.is_null() || (bo.2).1.is_null())
                            && opt =>
                    {
                        let is_null =
                            emit_is_null(e, env, if (bo.1).1.is_null() { &bo.2 } else { &bo.1 })?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![is_null, instr::jmpz(label.clone())]),
                            ),
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    E::Binop(bo)
                        if bo.0.is_diff2() && ((bo.1).1.is_null() || (bo.2).1.is_null()) && opt =>
                    {
                        let is_null =
                            emit_is_null(e, env, if (bo.1).1.is_null() { &bo.2 } else { &bo.1 })?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                pos,
                                InstrSeq::gather(vec![is_null, instr::jmpnz(label.clone())]),
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
                                InstrSeq::gather(vec![instr, instr::jmpz(label.clone())]),
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

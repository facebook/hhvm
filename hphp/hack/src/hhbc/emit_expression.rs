// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(unused_variables, dead_code)]

use ast_class_expr_rust::ClassExpr;
use ast_constant_folder_rust as ast_constant_folder;
use ast_scope_rust::Scope;
use emit_fatal_rust as emit_fatal;
use emit_symbol_refs_rust as emit_symbol_refs;
use emit_type_constant_rust as emit_type_constant;
use env::{emitter::Emitter, local, Env, Flags as EnvFlags};
use hhbc_ast_rust::*;
use hhbc_id_rust::{class, function, method, prop, Id};
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::{unrecoverable, Error::Unrecoverable, InstrSeq, Result};
use itertools::Itertools;
use label_rust::Label;
use naming_special_names_rust::{
    emitter_special_functions, fb, pseudo_consts, pseudo_functions, special_functions,
    special_idents, superglobals, user_attributes,
};
use ocaml_helper::int_of_str_opt;
use options::{CompilerFlags, HhvmFlags, LangFlags, Options};
use oxidized::{aast, aast_defs, ast as tast, ast_defs, local_id, pos::Pos};
use runtime::TypedValue;
use scope_rust::scope;

use std::{collections::BTreeMap, convert::TryInto, iter, str::FromStr};

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
        && Scope::has_this(&env.scope)
        && !Scope::is_toplevel(&env.scope)
}

mod inout_locals {
    use crate::*;
    use oxidized::{aast_defs::Lid, aast_visitor, aast_visitor::Node, ast as tast, ast_defs};
    use std::{collections::HashMap, marker::PhantomData};

    struct AliasInfo {
        first_inout: usize,
        last_write: usize,
        num_uses: usize,
    }

    impl Default for AliasInfo {
        fn default() -> Self {
            AliasInfo {
                first_inout: std::usize::MAX,
                last_write: std::usize::MIN,
                num_uses: 0,
            }
        }
    }

    impl AliasInfo {
        pub fn add_inout(&mut self, i: usize) {
            if i < self.first_inout {
                self.first_inout = i;
            }
        }

        pub fn add_write(&mut self, i: usize) {
            if i > self.last_write {
                self.last_write = i;
            }
        }

        pub fn add_use(&mut self) {
            self.num_uses += 1
        }

        pub fn in_range(&self, i: usize) -> bool {
            i > self.first_inout || i <= self.last_write
        }

        pub fn has_single_ref(&self) -> bool {
            self.num_uses < 2
        }
    }

    type AliasInfoMap = HashMap<String, AliasInfo>;

    fn add_write(name: String, i: usize, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_write(i);
    }

    fn add_inout(name: String, i: usize, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_inout(i);
    }

    fn add_use(name: String, map: &mut AliasInfoMap) {
        map.entry(name).or_default().add_use();
    }

    // determines if value of a local 'name' that appear in parameter 'i'
    // should be saved to local because it might be overwritten later
    fn should_save_local_value(name: String, i: usize, aliases: &AliasInfoMap) -> bool {
        aliases.get(&name).map_or(false, |alias| alias.in_range(i))
    }

    fn should_move_local_value(local: &local::Type, aliases: &AliasInfoMap) -> bool {
        match local {
            local::Type::Named(name) => aliases
                .get(&**name)
                .map_or(true, |alias| alias.has_single_ref()),
            local::Type::Unnamed(_) => false,
        }
    }

    fn collect_written_variables(env: &Env, args: &Vec<tast::Expr>) -> AliasInfoMap {
        let mut acc = HashMap::new();
        args.iter()
            .enumerate()
            .for_each(|(i, arg)| handle_arg(env, true, i, arg, &mut acc));
        acc
    }

    fn handle_arg(env: &Env, is_top: bool, i: usize, arg: &tast::Expr, acc: &mut AliasInfoMap) {
        use tast::{Expr, Expr_};

        let Expr(_, e) = arg;
        match e {
            Expr_::Callconv(x) => {
                if let (ast_defs::ParamKind::Pinout, Expr(_, Expr_::Lvar(lid))) = &**x {
                    let Lid(_, lid) = &**lid;
                    if !is_local_this(env, &lid) {
                        add_use(lid.1.to_string(), acc);
                        if is_top {
                            add_inout(lid.1.to_string(), i, acc);
                        } else {
                            add_write(lid.1.to_string(), i, acc);
                        }
                    }
                }
            }
            Expr_::Lvar(lid) => {
                let Lid(_, (_, id)) = &**lid;
                add_use(id.to_string(), acc);
            }
            _ => {
                // dive into argument value
                aast_visitor::visit(
                    &mut Visitor {
                        phantom: PhantomData,
                    },
                    &mut Ctx { state: acc, env, i },
                    arg,
                );
            }
        }
    }

    struct Visitor<'a> {
        phantom: PhantomData<&'a str>,
    }

    pub struct Ctx<'a> {
        state: &'a mut AliasInfoMap,
        env: &'a Env<'a>,
        i: usize,
    }

    impl<'a> aast_visitor::Visitor for Visitor<'a> {
        type Context = Ctx<'a>;
        type Ex = ast_defs::Pos;
        type Fb = ();
        type En = ();
        type Hi = ();

        fn object(
            &mut self,
        ) -> &mut dyn aast_visitor::Visitor<
            Context = Self::Context,
            Ex = Self::Ex,
            Fb = Self::Fb,
            En = Self::En,
            Hi = Self::Hi,
        > {
            self
        }

        fn visit_expr_(&mut self, c: &mut Self::Context, p: &tast::Expr_) {
            p.recurse(c, self.object());
            match p {
                tast::Expr_::Binop(expr) => {
                    let (bop, left, _) = &**expr;
                    if let ast_defs::Bop::Eq(_) = bop {
                        collect_lvars_hs(c, left)
                    }
                }
                tast::Expr_::Unop(expr) => {
                    let (uop, e) = &**expr;
                    match uop {
                        ast_defs::Uop::Uincr | ast_defs::Uop::Udecr => collect_lvars_hs(c, e),
                        _ => (),
                    }
                }
                tast::Expr_::Lvar(expr) => {
                    let Lid(_, (_, id)) = &**expr;
                    add_use(id.to_string(), &mut c.state);
                }
                tast::Expr_::Call(expr) => {
                    let (_, _, _, args, uarg) = &**expr;
                    args.iter()
                        .for_each(|arg| handle_arg(&c.env, false, c.i, arg, &mut c.state));
                    uarg.as_ref()
                        .map(|arg| handle_arg(&c.env, false, c.i, arg, &mut c.state));
                }
                _ => (),
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
            _ => (),
        }
    }
}

pub fn get_type_structure_for_hint(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hint: &aast::Hint,
) -> InstrSeq {
    let _tv =
        emit_type_constant::hint_to_type_constant(opts, tparams, targ_map, &hint, false, false);
    unimplemented!("TODO(hrust) after porting most of emit_adata")
}

pub struct Setrange {
    pub op: SetrangeOp,
    pub size: usize,
    pub vec: bool,
}

/// kind of value stored in local
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
    base_stack_size: usize,
    cls_stack_size: usize,
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
        | Expr_::String(_)
        | Expr_::Int(_)
        | Expr_::Null
        | Expr_::False
        | Expr_::True => {
            let v = ast_constant_folder::expr_to_typed_value(emitter, &env.namespace, expression)
                .map_err(|_| unrecoverable("expr_to_typed_value failed"))?;
            emit_pos_then(emitter, pos, InstrSeq::make_typedvalue(v))
        }
        Expr_::PrefixedString(e) => emit_expr(emitter, env, &e.1),
        Expr_::ParenthesizedExpr(e) => emit_expr(emitter, env, e),
        Expr_::Lvar(e) => {
            let Lid(pos, _) = &**e;
            Ok(InstrSeq::gather(vec![
                emit_pos(emitter, pos)?,
                emit_local(emitter, env, BareThisOp::Notice, e)?,
            ]))
        }
        Expr_::ClassConst(e) => emit_class_const(env, pos, e),
        Expr_::Unop(e) => emit_unop(emitter, env, pos, e),
        Expr_::Binop(e) => emit_binop(emitter, env, pos, e),
        Expr_::Pipe(e) => emit_pipe(env, e),
        Expr_::Is(is_expr) => {
            let (e, h) = &**is_expr;
            Ok(InstrSeq::gather(vec![
                emit_expr(emitter, env, e)?,
                emit_is_hint(env, pos, h)?,
            ]))
        }
        Expr_::As(e) => emit_as(env, pos, e),
        Expr_::Cast(e) => emit_cast(env, pos, e),
        Expr_::Eif(e) => emit_conditional_expr(env, pos, e),
        Expr_::ExprList(es) => Ok(InstrSeq::gather(
            es.iter()
                .map(|e| emit_expr(emitter, env, e))
                .collect::<Result<Vec<_>>>()?,
        )),
        Expr_::ArrayGet(e) => {
            let (base_expr, opt_elem_expr) = &**e;
            match (base_expr.lvar_name(), opt_elem_expr) {
                (Some(name), Some(e)) if name == superglobals::GLOBALS => {
                    Ok(InstrSeq::gather(vec![
                        emit_expr(emitter, env, e)?,
                        emit_pos(emitter, pos)?,
                        InstrSeq::make_cgetg(),
                    ]))
                }
                _ => emit_array_get(env, pos, QueryOp::CGet, e),
            }
        }
        Expr_::ObjGet(e) => emit_obj_get(env, pos, QueryOp::CGet, e),
        Expr_::Call(c) => emit_call_expr(emitter, env, pos, None, c),
        Expr_::New(e) => emit_new(emitter, env, pos, e),
        Expr_::Record(e) => emit_record(env, pos, e),
        Expr_::Array(es) => {
            let c = emit_collection(emitter, env, expression, es, None)?;
            emit_pos_then(emitter, pos, c)
        }
        Expr_::Darray(e) => {
            let instrs = emit_collection(emitter, env, expression, &mk_afkvalues(&e.1), None)?;
            emit_pos_then(emitter, pos, instrs)
        }
        Expr_::Varray(e) => {
            let instrs = emit_collection(emitter, env, expression, &mk_afvalues(&e.1), None)?;
            emit_pos_then(emitter, pos, instrs)
        }
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
            let (e1, e2) = (**e).to_owned();
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
        Expr_::Clone(e) => {
            let clone = emit_clone(emitter, env, e)?;
            emit_pos_then(emitter, pos, clone)
        }
        Expr_::Shape(e) => emit_pos_then(emitter, pos, emit_shape(env, expression, e)?),
        Expr_::Await(e) => emit_await(emitter, env, pos, e),
        Expr_::Yield(e) => emit_yield(emitter, env, pos, e),
        Expr_::Efun(e) => emit_pos_then(emitter, pos, emit_lambda(env, e)?),
        Expr_::ClassGet(e) => emit_class_get(env, QueryOp::CGet, e),
        Expr_::String2(es) => emit_string2(emitter, env, pos, es),
        Expr_::BracedExpr(e) => emit_expr(emitter, env, e),
        Expr_::Id(e) => {
            let instrs = emit_id(emitter, env, e)?;
            emit_pos_then(emitter, pos, instrs)
        }
        Expr_::Xml(e) => emit_xhp(env, pos, e),
        Expr_::Callconv(e) => Err(Unrecoverable(
            "emit_callconv: This should have been caught at emit_arg".into(),
        )),
        Expr_::Import(e) => emit_import(env, pos, e),
        Expr_::Omitted => Ok(InstrSeq::Empty),
        Expr_::YieldBreak => Err(Unrecoverable(
            "yield break should be in statement position".into(),
        )),
        Expr_::YieldFrom(_) => Err(Unrecoverable("complex yield_from expression".into())),
        Expr_::Lfun(_) => Err(Unrecoverable(
            "expected Lfun to be converted to Efun during closure conversion emit_expr".into(),
        )),
        _ => unimplemented!("TODO(hrust)"),
    }
}

fn emit_exprs(e: &mut Emitter, env: &Env, exprs: &[tast::Expr]) -> Result {
    if exprs.is_empty() {
        Ok(InstrSeq::Empty)
    } else {
        Ok(InstrSeq::gather(
            exprs
                .iter()
                .map(|expr| emit_expr(e, env, expr))
                .collect::<Result<Vec<_>>>()?,
        ))
    }
}

fn emit_pos_then(emitter: &Emitter, pos: &Pos, instrs: InstrSeq) -> Result {
    Ok(emit_pos_rust::emit_pos_then(emitter, pos, instrs))
}

fn emit_pos(emitter: &Emitter, pos: &Pos) -> Result {
    Ok(emit_pos_rust::emit_pos(emitter, pos))
}

fn emit_id(emitter: &mut Emitter, env: &Env, id: &tast::Sid) -> Result {
    use pseudo_consts::*;
    use InstructLitConst::*;

    let ast_defs::Id(p, s) = id;
    let res = match s.as_str() {
        G__FILE__ => InstrSeq::make_lit_const(File),
        G__DIR__ => InstrSeq::make_lit_const(Dir),
        G__METHOD__ => InstrSeq::make_lit_const(Method),
        G__FUNCTION_CREDENTIAL__ => InstrSeq::make_lit_const(FuncCred),
        G__CLASS__ => InstrSeq::gather(vec![InstrSeq::make_self(), InstrSeq::make_classname()]),
        G__COMPILER_FRONTEND__ => InstrSeq::make_string("hackc"),
        G__LINE__ => InstrSeq::make_int(p.info_pos_extended().1.try_into().map_err(|_| {
            emit_fatal::raise_fatal_parse(p, "error converting end of line from usize to isize")
        })?),
        G__NAMESPACE__ => InstrSeq::make_string(env.namespace.name.as_ref().map_or("", |s| &s[..])),
        EXIT | DIE => return emit_exit(emitter, env, None),
        _ => {
            //panic!("TODO: uncomment after D19350786 lands")
            //let cid: ConstId = r#const::Type::from_ast_name(&s);
            let cid: ConstId = string_utils::strip_global_ns(&s).to_string().into();
            emit_symbol_refs::State::add_constant(emitter, cid.clone());
            return emit_pos_then(emitter, p, InstrSeq::make_lit_const(CnsE(cid)));
        }
    };
    Ok(res)
}

fn emit_exit(emitter: &mut Emitter, env: &Env, expr_opt: Option<&tast::Expr>) -> Result {
    Ok(InstrSeq::gather(vec![
        expr_opt.map_or_else(|| Ok(InstrSeq::make_int(0)), |e| emit_expr(emitter, env, e))?,
        InstrSeq::make_exit(),
    ]))
}

fn emit_xhp(
    env: &Env,
    pos: &Pos,
    (id, attributes, children): &(tast::Sid, Vec<tast::XhpAttribute>, Vec<tast::Expr>),
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_yield(e: &mut Emitter, env: &Env, pos: &Pos, af: &tast::Afield) -> Result {
    Ok(match af {
        tast::Afield::AFvalue(v) => InstrSeq::gather(vec![
            emit_expr(e, env, v)?,
            emit_pos(e, pos)?,
            InstrSeq::make_yield(),
        ]),
        tast::Afield::AFkvalue(k, v) => InstrSeq::gather(vec![
            emit_expr(e, env, k)?,
            emit_expr(e, env, v)?,
            emit_pos(e, pos)?,
            InstrSeq::make_yieldk(),
        ]),
    })
}

fn emit_import(env: &Env, pos: &Pos, (flavor, expr): &(tast::ImportFlavor, tast::Expr)) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_string2(e: &mut Emitter, env: &Env, pos: &Pos, es: &Vec<tast::Expr>) -> Result {
    if es.is_empty() {
        Err(unrecoverable("String2 with zero araguments is impossible"))
    } else if es.len() == 1 {
        Ok(InstrSeq::gather(vec![
            emit_expr(e, env, &es[0])?,
            emit_pos(e, pos)?,
            InstrSeq::make_cast_string(),
        ]))
    } else {
        Ok(InstrSeq::gather(vec![
            emit_two_exprs(e, env, &es[0].0, &es[0], &es[1])?,
            emit_pos(e, pos)?,
            InstrSeq::make_concat(),
            InstrSeq::gather(
                (&es[2..])
                    .iter()
                    .map(|expr| {
                        Ok(InstrSeq::gather(vec![
                            emit_expr(e, env, expr)?,
                            emit_pos(e, pos)?,
                            InstrSeq::make_concat(),
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
        InstrSeq::make_clone(),
    ]))
}

fn emit_lambda(env: &Env, (fndef, ids): &(tast::Fun_, Vec<aast_defs::Lid>)) -> Result {
    unimplemented!("TODO(hrust)")
}

pub fn emit_await(emitter: &mut Emitter, env: &Env, pos: &Pos, expr: &tast::Expr) -> Result {
    let tast::Expr(_, e) = expr;
    let cant_inline_gen_functions = emitter
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION);
    match e.as_call() {
        Some((_, tast::Expr(_, tast::Expr_::Id(id)), _, args, None))
            if (cant_inline_gen_functions
                && args.len() == 1
                && string_utils::strip_global_ns(&(*id.1)) == "gena") =>
        {
            return inline_gena_call(emitter, env, &args[0])
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
                emit_pos(emitter, pos)?,
                InstrSeq::make_dup(),
                InstrSeq::make_istypec(IstypeOp::OpNull),
                InstrSeq::make_jmpnz(after_await.clone()),
                InstrSeq::make_await(),
                InstrSeq::make_label(after_await),
            ]))
        }
    }
}

fn hack_arr_dv_arrs(opts: &Options) -> bool {
    opts.hhvm.flags.contains(HhvmFlags::HACK_ARR_DV_ARRS)
}

fn inline_gena_call(emitter: &mut Emitter, env: &Env, arg: &tast::Expr) -> Result {
    let load_arr = emit_expr(emitter, env, arg)?;
    let async_eager_label = emitter.label_gen_mut().next_regular();
    let hack_arr_dv_arrs = hack_arr_dv_arrs(emitter.options());

    scope::with_unnamed_local(emitter, |e, arr_local| {
        let before = InstrSeq::gather(vec![
            load_arr,
            if hack_arr_dv_arrs {
                InstrSeq::make_cast_dict()
            } else {
                InstrSeq::make_cast_darray()
            },
            InstrSeq::make_popl(arr_local.clone()),
        ]);

        let inner = InstrSeq::gather(vec![
            InstrSeq::make_nulluninit(),
            InstrSeq::make_nulluninit(),
            InstrSeq::make_nulluninit(),
            InstrSeq::make_cgetl(arr_local.clone()),
            InstrSeq::make_fcallclsmethodd(
                FcallArgs::new(
                    FcallFlags::default(),
                    1,
                    vec![],
                    Some(async_eager_label.clone()),
                    1,
                ),
                method::from_raw_string(if hack_arr_dv_arrs {
                    "fromDict"
                } else {
                    "fromArray"
                }),
                class::from_raw_string("HH\\AwaitAllWaitHandle"),
            ),
            InstrSeq::make_await(),
            InstrSeq::make_label(async_eager_label.clone()),
            InstrSeq::make_popc(),
            emit_iter(
                e,
                InstrSeq::make_cgetl(arr_local.clone()),
                |key_local, val_local| {
                    InstrSeq::gather(vec![
                        InstrSeq::make_cgetl(val_local),
                        InstrSeq::make_whresult(),
                        InstrSeq::make_basel(arr_local.clone(), MemberOpMode::Define),
                        InstrSeq::make_setm(0, MemberKey::EL(key_local)),
                        InstrSeq::make_popc(),
                    ])
                },
            )?,
        ]);

        let after = InstrSeq::make_pushl(arr_local);

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
            InstrSeq::make_iterinit(iter_args.clone(), loop_end.clone()),
        ]);
        let iterate = InstrSeq::gather(vec![
            InstrSeq::make_label(loop_next.clone()),
            f(val_id.clone(), key_id.clone()),
            InstrSeq::make_iternext(iter_args, loop_next),
        ]);
        let iter_done = InstrSeq::gather(vec![
            InstrSeq::make_unsetl(val_id),
            InstrSeq::make_unsetl(key_id),
            InstrSeq::make_label(loop_end),
        ]);
        Ok((iter_init, iterate, iter_done))
    })
}

fn emit_shape(
    env: &Env,
    expr: &tast::Expr,
    fl: &Vec<(ast_defs::ShapeFieldName, tast::Expr)>,
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_vec_collection(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    fields: &Vec<tast::Afield>,
) -> Result {
    match ast_constant_folder::vec_to_typed_value(e, &env.namespace, pos, fields) {
        Ok(tv) => emit_static_collection(e, env, None, pos, tv),
        Err(_) => emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVecArray),
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
        if fields.is_empty() {
            emit_pos_then(e, pos, InstrSeq::make_newcol(collection_type))
        } else {
            Ok(InstrSeq::gather(vec![
                emit_vec_collection(e, env, pos, fields)?,
                InstrSeq::make_colfromarray(collection_type),
            ]))
        }
    };
    let emit_map_or_set = |e: &mut Emitter, collection_type| {
        if fields.is_empty() {
            emit_pos_then(e, pos, InstrSeq::make_newcol(collection_type))
        } else {
            emit_collection(e, env, expr, fields, Some(collection_type))
        }
    };
    use CollectionType as C;
    match collection_type {
        C::Dict | C::Vec | C::Keyset => {
            let instr = emit_collection(e, env, expr, fields, None)?;
            emit_pos_then(e, pos, instr)
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
            InstrSeq::make_new_pair(),
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
            )))
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
        true, /*allow_map*/
    ) {
        Ok(tv) => emit_static_collection(e, env, transform_to_collection, pos, tv),
        Err(_) => emit_dynamic_collection(e, env, expr, fields),
    }
}

fn emit_static_collection(
    e: &mut Emitter,
    env: &Env,
    transform_to_collection: Option<CollectionType>,
    pos: &Pos,
    tv: TypedValue,
) -> Result {
    let arrprov_enabled = e.options().hhvm.flags.contains(HhvmFlags::ARRAY_PROVENANCE);
    let transform_instr = match transform_to_collection {
        Some(collection_type) => InstrSeq::make_colfromarray(collection_type),
        _ => InstrSeq::Empty,
    };
    Ok(
        if arrprov_enabled && env.scope.has_function_attribute("__ProvenanceSkipFrame") {
            InstrSeq::gather(vec![
                emit_pos(e, pos)?,
                InstrSeq::make_nulluninit(),
                InstrSeq::make_nulluninit(),
                InstrSeq::make_nulluninit(),
                InstrSeq::make_typedvalue(tv),
                InstrSeq::make_fcallfuncd(
                    FcallArgs::new(FcallFlags::default(), 0, vec![], None, 1),
                    function::from_raw_string("HH\\tag_provenance_here"),
                ),
                transform_instr,
            ])
        } else {
            InstrSeq::gather(vec![
                emit_pos(e, pos)?,
                InstrSeq::make_typedvalue(tv),
                transform_instr,
            ])
        },
    )
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
            emit_pos(e, pos)?,
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
        CollectionType::Dict | CollectionType::Array => {
            (InstrSeq::Empty, InstrSeq::make_add_new_elemc())
        }
        _ => (
            InstrSeq::make_colfromarray(ctype),
            InstrSeq::gather(vec![InstrSeq::make_dup(), InstrSeq::make_add_elemc()]),
        ),
    };
    let emitted_pos = emit_pos(e, pos)?;
    Ok(InstrSeq::gather(vec![
        emitted_pos.clone(),
        InstrSeq::make_lit_const(constructor),
        InstrSeq::gather(
            fields
                .iter()
                .map(|f| {
                    expr_and_new(
                        e,
                        env,
                        pos,
                        add_elem_instr.clone(),
                        InstrSeq::make_add_elemc(),
                        f,
                    )
                })
                .collect::<Result<_>>()?,
        ),
        emitted_pos,
        transform_instr,
    ]))
}

fn is_struct_init(
    e: &mut Emitter,
    env: &Env,
    fields: &[tast::Afield],
    allow_numerics: bool,
) -> bool {
    let mut are_all_keys_non_numeric_strings = true;
    let mut has_duplicate_keys = false;
    let mut uniq_keys = std::collections::HashSet::<String>::new();
    for f in fields.iter() {
        if let tast::Afield::AFkvalue(key, _) = f {
            // TODO(hrust): if key is String, don't clone and call fold_expr
            let mut key = key.clone();
            ast_constant_folder::fold_expr(&mut key, e, &env.namespace);
            if let tast::Expr(_, tast::Expr_::String(s)) = key {
                are_all_keys_non_numeric_strings = are_all_keys_non_numeric_strings
                    && !i64::from_str(&s).is_ok()
                    && !f64::from_str(&s).is_ok();
                has_duplicate_keys = has_duplicate_keys || !uniq_keys.insert(s);
            }
            if !are_all_keys_non_numeric_strings && has_duplicate_keys {
                break;
            }
        }
    }
    let num_keys = fields.len();
    let limit = *(e.options().max_array_elem_size_on_the_stack.get()) as usize;
    (allow_numerics || are_all_keys_non_numeric_strings)
        && !has_duplicate_keys
        && num_keys <= limit
        && num_keys != 0
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
                E(_, E_::String(s)) => Ok((s.clone(), emit_expr(e, env, v)?)),
                _ => {
                    let mut k = k.clone();
                    ast_constant_folder::fold_expr(&mut k, e, &env.namespace);
                    match k {
                        E(_, E_::String(s)) => Ok((s.clone(), emit_expr(e, env, v)?)),
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
        emit_pos(e, pos)?,
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
        if is_struct_init(e, env, fields, true) {
            emit_struct_array(e, env, pos, fields, |_, x| {
                Ok(InstrSeq::make_newstructdict(x))
            })
        } else {
            let ctor = InstructLitConst::NewDictArray(count as isize);
            emit_keyvalue_collection(e, env, pos, fields, CollectionType::Dict, ctor)
        }
    };
    let emit_collection_helper = |e: &mut Emitter, ctype| {
        if is_struct_init(e, env, fields, true) {
            Ok(InstrSeq::gather(vec![
                emit_struct_array(e, env, pos, fields, |_, x| {
                    Ok(InstrSeq::make_newstructdict(x))
                })?,
                emit_pos(e, pos)?,
                InstrSeq::make_colfromarray(ctype),
            ]))
        } else {
            let ctor = InstructLitConst::NewDictArray(count as isize);
            emit_keyvalue_collection(e, env, pos, fields, ctype, ctor)
        }
    };
    use tast::Expr_ as E_;
    match &expr.1 {
        E_::ValCollection(v) if v.0 == tast::VcKind::Vec => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVecArray)
        }
        E_::Collection(v) if (v.0).1 == "vec" => {
            emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewVecArray)
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
            let hack_arr_dv_arrs = hack_arr_dv_arrs(e.options());
            emit_value_only_collection(e, env, pos, fields, |n| {
                if hack_arr_dv_arrs {
                    InstructLitConst::NewVecArray(n)
                } else {
                    InstructLitConst::NewVArray(n)
                }
            })
        }
        E_::Darray(_) => {
            if is_struct_init(e, env, fields, false /* allow_numerics */) {
                let hack_arr_dv_arrs = hack_arr_dv_arrs(e.options());
                emit_struct_array(e, env, pos, fields, |e, arg| {
                    let instr = if hack_arr_dv_arrs {
                        InstrSeq::make_newstructdict(arg)
                    } else {
                        InstrSeq::make_newstructdarray(arg)
                    };
                    emit_pos_then(e, pos, instr)
                })
            } else {
                let constr = if hack_arr_dv_arrs(e.options()) {
                    InstructLitConst::NewDictArray(count as isize)
                } else {
                    InstructLitConst::NewDArray(count as isize)
                };
                emit_keyvalue_collection(e, env, pos, fields, CollectionType::Array, constr)
            }
        }
        _ => {
            if is_packed_init(e.options(), fields, true /* hack_arr_compat */) {
                emit_value_only_collection(e, env, pos, fields, InstructLitConst::NewPackedArray)
            } else if is_struct_init(e, env, fields, false /* allow_numerics */) {
                emit_struct_array(e, env, pos, fields, |_, x| {
                    Ok(InstrSeq::make_newstructarray(x))
                })
            } else if is_packed_init(e.options(), fields, false /* hack_arr_compat*/) {
                let constr = InstructLitConst::NewArray(count as isize);
                emit_keyvalue_collection(e, env, pos, fields, CollectionType::Array, constr)
            } else {
                let constr = InstructLitConst::NewMixedArray(count as isize);
                emit_keyvalue_collection(e, env, pos, fields, CollectionType::Array, constr)
            }
        }
    }
}

/// is_packed_init() returns true if this expression list looks like an
/// array with no keys and no ref values
fn is_packed_init(opts: &Options, es: &[tast::Afield], hack_arr_compat: bool) -> bool {
    let is_only_values = es.iter().all(|f| !f.is_afkvalue());
    let has_bool_keys = es.iter().any(|f| {
        f.as_afkvalue()
            .map(|(tast::Expr(_, k), _)| k.is_true() || k.is_false())
            .is_some()
    });
    let keys_are_zero_indexed_properly_formed = es.iter().enumerate().all(|(i, f)| {
        use tast::{Afield as A, Expr as E, Expr_ as E_};
        match f {
            A::AFkvalue(E(_, E_::Int(k)), _) => int_of_str_opt(k).unwrap() == i as i64, // already checked in lowerer
            // arrays with int-like string keys are still considered packed
            // and should be emitted via NewArray
            A::AFkvalue(E(_, E_::String(s)), _) => {
                int_of_str_opt(s).map_or(false, |s| s == i as i64)
            }
            A::AFkvalue(E(_, E_::True), _) => i == 1,
            A::AFkvalue(E(_, E_::False), _) => i == 0,
            A::AFvalue(_) => true,
            _ => false,
        }
    });
    (is_only_values || keys_are_zero_indexed_properly_formed)
        && (!(has_bool_keys
            && hack_arr_compat
            && opts.hhvm.flags.contains(HhvmFlags::HACK_ARR_COMPAT_NOTICES)))
        && !es.is_empty()
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
            emit_pos(e, pos)?,
            InstrSeq::make_lit_const(constructor(exprs.len() as isize)),
        ]))
    };
    let outofline = |e: &mut Emitter, exprs: &[tast::Afield]| -> Result {
        Ok(InstrSeq::gather(
            exprs
                .iter()
                .map(|f| {
                    Ok(InstrSeq::gather(vec![
                        emit_expr(e, env, f.value())?,
                        InstrSeq::make_add_new_elemc(),
                    ]))
                })
                .collect::<Result<_>>()?,
        ))
    };
    let (x1, fields) = fields.split_at(std::cmp::min(fields.len(), limit));
    let (x2, _) = fields.split_at(std::cmp::min(fields.len(), limit));
    Ok(match (x1, x2) {
        ([], []) => InstrSeq::Empty,
        (_, []) => inline(e, x1)?,
        _ => InstrSeq::gather(vec![inline(e, x1)?, outofline(e, x2)?]),
    })
}

fn emit_record(
    env: &Env,
    pos: &Pos,
    (cid, is_array, es): &(tast::Sid, bool, Vec<(tast::Expr, tast::Expr)>),
) -> Result {
    let es = mk_afkvalues(es);
    unimplemented!("TODO(hrust)")
}

fn emit_call_isset_exprs(e: &mut Emitter, env: &Env, pos: &Pos, exprs: &[tast::Expr]) -> Result {
    unimplemented!()
}

fn emit_idx(e: &mut Emitter, env: &Env, pos: &Pos, es: &[tast::Expr]) -> Result {
    let default = if es.len() == 2 {
        InstrSeq::make_null()
    } else {
        InstrSeq::Empty
    };
    Ok(InstrSeq::gather(vec![
        emit_exprs(e, env, es)?,
        emit_pos(e, pos)?,
        default,
        InstrSeq::make_idx(),
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
    let fcall_args = get_fcall_args(args, uarg, async_eager_label, false);
    let FcallArgs(_, _, num_ret, _, _) = &fcall_args;
    let num_uninit = num_ret - 1;
    let default = scope::with_unnamed_locals(e, |e| {
        let (lhs, fcall) = emit_call_lhs_and_fcall(e, env, expr, fcall_args, targs)?;
        let (args, inout_setters) = emit_args_inout_setters(e, env, args)?;
        let uargs = uarg.map_or(Ok(InstrSeq::Empty), |uarg| emit_expr(e, env, uarg))?;
        Ok((
            InstrSeq::Empty,
            InstrSeq::gather(vec![
                InstrSeq::gather(
                    iter::repeat(InstrSeq::make_nulluninit())
                        .take(num_uninit)
                        .collect::<Vec<_>>(),
                ),
                lhs,
                args,
                uargs,
                emit_pos(e, pos)?,
                fcall,
                inout_setters,
            ]),
            InstrSeq::Empty,
        ))
    })?;
    expr.1
        .as_id()
        .and_then(|ast_defs::Id(_, id)| {
            emit_special_function(e, env, pos, &expr.0, &id, args, uarg, &default).transpose()
        })
        .unwrap_or(Ok(default))
}

fn emit_reified_targs(e: &mut Emitter, env: &Env, pos: &Pos, targs: &[&tast::Hint]) -> Result {
    unimplemented!()
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

fn has_non_tparam_generics_targs(env: &Env, targs: &[tast::Targ]) -> bool {
    let erased_tparams = get_erased_tparams(env);
    targs.iter().any(|targ| {
        (targ.1)
            .1
            .as_happly()
            .map_or(true, |(id, _)| !erased_tparams.contains(&id.1.as_str()))
    })
}

fn emit_call_lhs_and_fcall(
    e: &mut Emitter,
    env: &Env,
    expr: &tast::Expr,
    mut fcall_args: FcallArgs,
    targs: &[tast::Targ],
) -> Result<(InstrSeq, InstrSeq)> {
    let tast::Expr(pos, expr_) = expr;
    use tast::Expr_ as E_;

    let emit_generics = |e, env, fcall_args: &mut FcallArgs| {
        let does_not_have_non_tparam_generics = !has_non_tparam_generics_targs(env, targs);
        if does_not_have_non_tparam_generics {
            Ok(InstrSeq::Empty)
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

    match expr_ {
        E_::ObjGet(_) => unimplemented!(),
        E_::ClassConst(_) => unimplemented!(),
        E_::ClassGet(_) => unimplemented!(),
        E_::Id(id) => {
            let FcallArgs(flags, num_args, _, _, _) = fcall_args;
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
                InstrSeq::gather(vec![
                    InstrSeq::make_nulluninit(),
                    InstrSeq::make_nulluninit(),
                    InstrSeq::make_nulluninit(),
                ]),
                InstrSeq::gather(vec![generics, InstrSeq::make_fcallfuncd(fcall_args, fq_id)]),
            ))
        }
        E_::String(s) => unimplemented!(),
        _ => {
            let tmp = e.local_gen_mut().get_unnamed();
            Ok((
                InstrSeq::gather(vec![
                    InstrSeq::make_nulluninit(),
                    InstrSeq::make_nulluninit(),
                    InstrSeq::make_nulluninit(),
                    emit_expr(e, env, expr)?,
                    InstrSeq::make_popl(tmp.clone()),
                ]),
                InstrSeq::gather(vec![
                    InstrSeq::make_pushl(tmp),
                    InstrSeq::make_fcallfunc(fcall_args),
                ]),
            ))
        }
    }
}

fn emit_args_inout_setters(
    e: &mut Emitter,
    env: &Env,
    args: &[tast::Expr],
) -> Result<(InstrSeq, InstrSeq)> {
    fn emit_arg_and_inout_setter(
        e: &mut Emitter,
        env: &Env,
        i: usize,
        arg: &tast::Expr,
    ) -> Result<(InstrSeq, InstrSeq)> {
        use tast::Expr_ as E_;
        match &arg.1 {
            E_::Callconv(cc) => {
                match &(cc.1).1 {
                    // inout $var
                    E_::Lvar(l) => unimplemented!(),
                    // inout $arr[...][...]
                    E_::ArrayGet(ag) => unimplemented!(),
                    _ => Err(unrecoverable(
                        "emit_arg_and_inout_setter: Unexpected inout expression type",
                    )),
                }
            }
            _ => Ok((emit_expr(e, env, arg)?, InstrSeq::Empty)),
        }
    }
    let (instr_args, instr_setters): (Vec<InstrSeq>, Vec<InstrSeq>) = args
        .iter()
        .enumerate()
        .map(|(i, arg)| emit_arg_and_inout_setter(e, env, i, arg))
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
                InstrSeq::make_popl(retval.clone()),
                instr_setters,
                InstrSeq::make_pushl(retval),
            ]),
        ))
    } else {
        Ok((instr_args, InstrSeq::Empty))
    }
}

fn get_fcall_args(
    args: &[tast::Expr],
    uarg: Option<&tast::Expr>,
    async_eager_label: Option<Label>,
    lock_while_unwinding: bool,
) -> FcallArgs {
    let num_args = args.len();
    let num_rets = 1 + args.iter().filter(|x| is_inout_arg(*x)).count();
    let mut flags = FcallFlags::default();
    flags.set(FcallFlags::HAS_UNPACK, uarg.is_some());
    flags.set(FcallFlags::LOCK_WHILE_UNWINDING, lock_while_unwinding);
    let inouts: Vec<bool> = args.iter().map(is_inout_arg).collect();
    FcallArgs::new(flags, num_rets, inouts, async_eager_label, num_args)
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
    annot: &Pos,
    id: &str,
    args: &[tast::Expr],
    uarg: Option<&tast::Expr>,
    default: &InstrSeq,
) -> Result<Option<InstrSeq>> {
    use tast::{Expr as E, Expr_ as E_};
    let nargs = args.len() + uarg.map_or(0, |_| 1);
    let fq = function::Type::from_ast_name(id);
    let lower_fq_name = fq.to_raw_string();
    match (lower_fq_name, args) {
        (id, _) if id == special_functions::ECHO => Ok(Some(InstrSeq::gather(
            args.iter()
                .enumerate()
                .map(|(i, arg)| {
                    Ok(InstrSeq::gather(vec![
                        emit_expr(e, env, arg)?,
                        emit_pos(e, pos)?,
                        InstrSeq::make_print(),
                        if i == nargs - 1 {
                            InstrSeq::Empty
                        } else {
                            InstrSeq::make_popc()
                        },
                    ]))
                })
                .collect::<Result<_>>()?,
        ))),
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
                annot.clone(),
                tast::Expr_::mk_call(
                    tast::CallType::Cnormal,
                    expr_id,
                    vec![],
                    args[1..].to_owned(),
                    uarg.cloned(),
                ),
            );
            Ok(Some(InstrSeq::gather(vec![
                emit_expr(e, env, &args[0])?,
                InstrSeq::make_jmpnz(l.clone()),
                emit_ignored_expr(e, env, &Pos::make_none(), &call)?,
                emit_fatal::emit_fatal_runtime(e, pos, "invariant_violation"),
                InstrSeq::make_label(l),
                InstrSeq::make_null(),
            ])))
        }
        ("assert", _) => {
            let l0 = e.label_gen_mut().next_regular();
            let l1 = e.label_gen_mut().next_regular();
            Ok(Some(InstrSeq::gather(vec![
                InstrSeq::make_string("zend.assertions"),
                InstrSeq::make_fcallbuiltin(1, 1, 0, "ini_get"),
                InstrSeq::make_int(0),
                InstrSeq::make_gt(),
                InstrSeq::make_jmpz(l0.clone()),
                default.clone(),
                InstrSeq::make_jmp(l1.clone()),
                InstrSeq::make_label(l0),
                InstrSeq::make_true(),
                InstrSeq::make_label(l1),
            ])))
        }
        ("HH\\sequence", &[]) => Ok(Some(InstrSeq::make_null())),
        ("HH\\sequence", args) => Ok(Some(InstrSeq::gather(
            args.iter()
                .map(|arg| emit_expr(e, env, arg))
                .collect::<Result<Vec<_>>>()?
                .into_iter()
                .intersperse(InstrSeq::make_popc())
                .collect::<Vec<_>>(),
        ))),
        ("class_exists", _) if nargs == 1 || nargs == 2 => unimplemented!(),
        ("trait_exists", _) if nargs == 1 || nargs == 2 => unimplemented!(),
        ("interface_exists", _) if nargs == 1 || nargs == 2 => unimplemented!(),
        ("exit", _) | ("die", _) if nargs == 0 || nargs == 1 => {
            Ok(Some(emit_exit(e, env, args.first())?))
        }
        ("HH\\fun", _) => unimplemented!(),
        ("__systemlib\\fun", _) => unimplemented!(),
        ("HH\\inst_meth", _) => unimplemented!(),
        ("HH\\class_meth", _) => unimplemented!(),
        ("HH\\global_set", _) => unimplemented!(),
        ("HH\\global_unset", _) => unimplemented!(),
        ("__hhvm_internal_whresult", &[E(_, E_::Lvar(ref _p))]) => unimplemented!(),
        ("__hhvm_internal_newlikearrayl", &[E(_, E_::Lvar(ref _p)), E(_, E_::Int(ref _n))]) => {
            unimplemented!()
        }
        _ => Ok(
            match (
                args,
                istype_op(e.options(), lower_fq_name),
                is_isexp_op(lower_fq_name),
            ) {
                (&[ref arg_expr], _, Some(ref h)) => Some(InstrSeq::gather(vec![
                    emit_expr(e, env, &arg_expr)?,
                    emit_is(e, env, pos, &h)?,
                ])),
                (&[E(_, E_::Lvar(ref arg_id))], Some(i), _)
                    if superglobals::is_any_global(arg_id.name()) =>
                {
                    Some(InstrSeq::gather(vec![
                        emit_local(e, env, BareThisOp::NoNotice, &arg_id)?,
                        emit_pos(e, pos)?,
                        InstrSeq::make_istypec(i),
                    ]))
                }
                (&[E(_, E_::Lvar(ref arg_id))], Some(i), _) if !is_local_this(env, &arg_id.1) => {
                    Some(InstrSeq::make_istypel(
                        get_local(e, env, &arg_id.0, &(arg_id.1).1)?,
                        i,
                    ))
                }
                (&[ref arg_expr], Some(i), _) => Some(InstrSeq::gather(vec![
                    emit_expr(e, env, &arg_expr)?,
                    emit_pos(e, pos)?,
                    InstrSeq::make_istypec(i),
                ])),
                _ => match get_call_builtin_func_info(e.options(), lower_fq_name) {
                    Some((nargs, i)) if nargs == args.len() => Some(InstrSeq::gather(vec![
                        emit_exprs(e, env, args)?,
                        emit_pos(e, pos)?,
                        InstrSeq::make_instr(i),
                    ])),
                    _ => None,
                },
            },
        ),
    }
}

fn get_call_builtin_func_info(opts: &Options, id: impl AsRef<str>) -> Option<(usize, Instruct)> {
    use {Instruct::*, InstructGet::*, InstructIsset::*, InstructMisc::*, InstructOperator::*};
    let hack_arr_dv_arrs = hack_arr_dv_arrs(opts);
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
        "HH\\varray" => Some((
            1,
            IOp(if hack_arr_dv_arrs {
                CastVec
            } else {
                CastVArray
            }),
        )),
        "HH\\darray" => Some((
            1,
            IOp(if hack_arr_dv_arrs {
                CastDict
            } else {
                CastDArray
            }),
        )),
        "HH\\global_get" => Some((1, IGet(CGetG))),
        "HH\\global_isset" => Some((1, IIsset(IssetG))),
        _ => None,
    }
}

fn emit_is(e: &mut Emitter, env: &Env, pos: &Pos, h: &tast::Hint) -> Result {
    unimplemented!()
}

fn istype_op(opts: &Options, id: impl AsRef<str>) -> Option<IstypeOp> {
    let widen_is_array = opts.hhvm.flags.contains(HhvmFlags::WIDEN_IS_ARRAY);
    let hack_arr_dv_arrs = hack_arr_dv_arrs(opts);
    use IstypeOp::*;
    match id.as_ref() {
        "is_int" | "is_integer" | "is_long" => Some(OpInt),
        "is_bool" => Some(OpBool),
        "is_float" | "is_real" | "is_double" => Some(OpDbl),
        "is_string" => Some(OpStr),
        "is_array" => Some(if widen_is_array { OpArrLike } else { OpArr }),
        "is_object" => Some(OpObj),
        "is_null" => Some(OpNull),
        "is_scalar" => Some(OpScalar),
        "HH\\is_keyset" => Some(OpKeyset),
        "HH\\is_dict" => Some(OpDict),
        "HH\\is_vec" => Some(OpVec),
        "HH\\is_varray" => Some(if hack_arr_dv_arrs { OpVec } else { OpVArray }),
        "HH\\is_darray" => Some(if hack_arr_dv_arrs { OpDict } else { OpDArray }),
        "HH\\is_any_array" => Some(OpArrLike),
        "HH\\is_class_meth" => Some(OpClsMeth),
        "HH\\is_fun" => Some(OpFunc),
        "HH\\is_php_array" => Some(OpPHPArr),
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
        emit_pos(e, pos)?,
        InstrSeq::make_eval(),
    ]))
}

fn emit_call_expr(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    async_eager_label: Option<Label>,
    (_, expr, targs, args, uarg): &(
        tast::CallType,
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
            let v = TypedValue::HhasAdata(data.into());
            emit_pos_then(e, pos, InstrSeq::make_typedvalue(v))
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
                emit_pos(e, pos)?,
                InstrSeq::make_popl(local::Type::Named("$86metadata".into())),
                InstrSeq::make_null(),
            ]))
        }
        (E_::Id(id), [], None)
            if id.1 == pseudo_functions::EXIT || id.1 == pseudo_functions::DIE =>
        {
            let exit = emit_exit(e, env, None)?;
            emit_pos_then(e, pos, exit)
        }
        (E_::Id(id), [arg1], None)
            if id.1 == pseudo_functions::EXIT || id.1 == pseudo_functions::DIE =>
        {
            let exit = emit_exit(e, env, Some(arg1))?;
            emit_pos_then(e, pos, exit)
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
            emit_pos_then(e, pos, instrs)
        }
    }
}

fn emit_reified_generic_instrs(e: &mut Emitter, pos: &Pos, is_fun: bool, index: usize) -> Result {
    let base = if is_fun {
        InstrSeq::make_basel(
            local::Type::Named(string_utils::reified::GENERICS_LOCAL_NAME.into()),
            MemberOpMode::Warn,
        )
    } else {
        InstrSeq::gather(vec![
            InstrSeq::make_checkthis(),
            InstrSeq::make_baseh(),
            InstrSeq::make_dim_warn_pt(prop::from_raw_string(string_utils::reified::PROP_NAME)),
        ])
    };
    emit_pos_then(
        e,
        pos,
        InstrSeq::gather(vec![
            base,
            InstrSeq::make_querym(0, QueryOp::CGet, MemberKey::EI(index.try_into().unwrap())),
        ]),
    )
}

fn emit_reified_type_opt(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    name: &str,
) -> Result<Option<InstrSeq>> {
    let is_in_lambda = env.scope.is_in_lambda();
    let cget_instr = |is_fun, i| {
        InstrSeq::make_cgetl(local::Type::Named(
            string_utils::reified::reified_generic_captured_name(is_fun, i),
        ))
    };
    let check = |is_soft| -> Result<()> {
        if is_soft {
            Err(emit_fatal::raise_fatal_parse(pos, format!(
                "{} is annotated to be a soft reified generic, it cannot be used until the __Soft annotation is removed",
                name
            )))
        } else {
            Ok(())
        }
    };
    let mut emit = |(i, is_soft), is_fun| {
        check(is_soft)?;
        Ok(Some(if is_in_lambda {
            cget_instr(is_fun, i)
        } else {
            emit_reified_generic_instrs(e, pos, is_fun, i)?
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
    emit_symbol_refs::State::add_class(e, cid.clone());
    InstrSeq::gather(vec![
        InstrSeq::make_string(cid.to_raw_string()),
        InstrSeq::make_classgetc(),
    ])
}

fn emit_load_class_ref(e: &mut Emitter, env: &Env, pos: &Pos, cexpr: ClassExpr) -> Result {
    let instrs = match cexpr {
        ClassExpr::Special(SpecialClsRef::Self_) => InstrSeq::make_self(),
        ClassExpr::Special(SpecialClsRef::Static) => InstrSeq::make_lateboundcls(),
        ClassExpr::Special(SpecialClsRef::Parent) => InstrSeq::make_parent(),
        ClassExpr::Id(id) => emit_known_class_id(e, &id),
        ClassExpr::Expr(expr) => InstrSeq::gather(vec![
            emit_pos(e, pos)?,
            emit_expr(e, env, &expr)?,
            InstrSeq::make_classgetc(),
        ]),
        ClassExpr::Reified(instrs) => {
            InstrSeq::gather(vec![emit_pos(e, pos)?, instrs, InstrSeq::make_classgetc()])
        }
    };
    emit_pos_then(e, pos, instrs)
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
) -> Result {
    if has_inout_arg(args) {
        return Err(unrecoverable("Unexpected inout arg in new expr"));
    }
    let resolve_self = true;
    use HasGenericsOp as H;
    let cexpr = ClassExpr::class_id_to_class_expr(e, false, resolve_self, &env.scope, cid);
    let (cexpr, has_generics) = match &cexpr {
        ClassExpr::Id(ast_defs::Id(_, name)) => match emit_reified_type_opt(e, env, pos, name)? {
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
    let newobj_instrs = match cexpr {
        ClassExpr::Id(ast_defs::Id(_, cname)) => {
            // TODO(hrust) enabel `let id = class::Type::from_ast_name(&cname);`,
            // `from_ast_name` should be able to accpet Cow<str>
            let id: class::Type = string_utils::strip_global_ns(&cname).to_string().into();
            emit_symbol_refs::State::add_class(e, id.clone());
            match has_generics {
                H::NoGenerics => {
                    InstrSeq::gather(vec![emit_pos(e, pos)?, InstrSeq::make_newobjd(id)])
                }
                H::HasGenerics => InstrSeq::gather(vec![
                    emit_pos(e, pos)?,
                    emit_reified_targs(
                        e,
                        env,
                        pos,
                        &targs.iter().map(|t| &t.1).collect::<Vec<_>>(),
                    )?,
                    InstrSeq::make_newobjrd(id),
                ]),
                H::MaybeGenerics => {
                    return Err(unrecoverable(
                        "Internal error: This case should have been transformed",
                    ))
                }
            }
        }
        ClassExpr::Special(cls_ref) => {
            InstrSeq::gather(vec![emit_pos(e, pos)?, InstrSeq::make_newobjs(cls_ref)])
        }
        ClassExpr::Reified(instrs) if has_generics == H::MaybeGenerics => InstrSeq::gather(vec![
            instrs,
            InstrSeq::make_classgetts(),
            InstrSeq::make_newobjr(),
        ]),
        _ => InstrSeq::gather(vec![
            emit_load_class_ref(e, env, pos, cexpr)?,
            InstrSeq::make_newobj(),
        ]),
    };
    scope::with_unnamed_locals(e, |e| {
        let (instr_args, _) = emit_args_inout_setters(e, env, args)?;
        let instr_uargs = match uarg {
            None => InstrSeq::Empty,
            Some(uarg) => emit_expr(e, env, uarg)?,
        };
        Ok((
            InstrSeq::Empty,
            InstrSeq::gather(vec![
                newobj_instrs,
                InstrSeq::make_dup(),
                InstrSeq::make_nulluninit(),
                InstrSeq::make_nulluninit(),
                instr_args,
                instr_uargs,
                emit_pos(e, pos)?,
                InstrSeq::make_fcallctor(get_fcall_args(args, uarg.as_ref(), None, true)),
                InstrSeq::make_popc(),
                InstrSeq::make_lockobj(),
            ]),
            InstrSeq::Empty,
        ))
    })
}

fn emit_obj_get(
    env: &Env,
    pos: &Pos,
    query_op: QueryOp,
    (expr, prop, nullflavor): &(tast::Expr, tast::Expr, ast_defs::OgNullFlavor),
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_array_get(
    env: &Env,
    pos: &Pos,
    query_op: QueryOp,
    (base_expr, opt_elem_expr): &(tast::Expr, Option<tast::Expr>),
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_class_get(
    env: &Env,
    query_op: QueryOp,
    (cid, cls_get_expr): &(tast::ClassId, tast::ClassGetExpr),
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_conditional_expr(
    env: &Env,
    pos: &Pos,
    (etest, etrue, efalse): &(tast::Expr, Option<tast::Expr>, tast::Expr),
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_local(e: &mut Emitter, env: &Env, notice: BareThisOp, lid: &aast_defs::Lid) -> Result {
    let tast::Lid(pos, id) = lid;
    let id_name = local_id::get_name(id);
    if superglobals::GLOBALS == id_name {
        Err(emit_fatal::raise_fatal_parse(
            pos,
            "Access $GLOBALS via wrappers",
        ))
    } else if superglobals::is_superglobal(id_name) {
        Ok(InstrSeq::gather(vec![
            InstrSeq::make_string(string_utils::locals::strip_dollar(id_name)),
            emit_pos(e, pos)?,
            InstrSeq::make_cgetg(),
        ]))
    } else {
        let local = get_local(e, env, pos, id_name)?;
        if is_local_this(env, id) && !env.flags.contains(EnvFlags::NEEDS_LOCAL_THIS) {
            emit_pos_then(e, pos, InstrSeq::make_barethis(notice))
        } else {
            Ok(InstrSeq::make_cgetl(local))
        }
    }
}

fn emit_class_const(env: &Env, pos: &Pos, (ci, id): &(tast::ClassId, ast_defs::Pstring)) -> Result {
    unimplemented!("TODO(hrust)")
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
            emit_pos_then(e, pos, from_unop(e.options(), uop)?)?,
        ])),
        U::Uplus | U::Uminus => Ok(InstrSeq::gather(vec![
            emit_pos(e, pos)?,
            InstrSeq::make_int(0),
            emit_expr(e, env, expr)?,
            emit_pos_then(e, pos, from_unop(e.options(), uop)?)?,
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
                emit_pos(e, pos)?,
                InstrSeq::make_silence_start(temp_local.clone()),
                {
                    let try_instrs = emit_expr(e, env, expr)?;
                    let catch_instrs = InstrSeq::gather(vec![
                        emit_pos(e, pos)?,
                        InstrSeq::make_silence_end(temp_local.clone()),
                    ]);
                    InstrSeq::create_try_catch(
                        e.label_gen_mut(),
                        None,
                        false, /* skip_throw */
                        try_instrs,
                        catch_instrs,
                    )
                },
                emit_pos(e, pos)?,
                InstrSeq::make_silence_end(temp_local),
            ]))
        }),
    }
}

fn unop_to_incdec_op(opts: &Options, op: &ast_defs::Uop) -> Result<IncdecOp> {
    let check_int_overflow = opts
        .hhvm
        .hack_lang_flags
        .contains(LangFlags::CHECK_INT_OVERFLOW);
    let if_check_or = |op1, op2| Ok(if check_int_overflow { op1 } else { op2 });
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
    let check_int_overflow = opts
        .hhvm
        .hack_lang_flags
        .contains(LangFlags::CHECK_INT_OVERFLOW);
    use ast_defs::Uop as U;
    Ok(match op {
        U::Utild => InstrSeq::make_bitnot(),
        U::Unot => InstrSeq::make_not(),
        U::Uplus => {
            if check_int_overflow {
                InstrSeq::make_addo()
            } else {
                InstrSeq::make_add()
            }
        }
        U::Uminus => {
            if check_int_overflow {
                InstrSeq::make_subo()
            } else {
                InstrSeq::make_sub()
            }
        }
        _ => {
            return Err(Unrecoverable(
                "this unary operation cannot be translated".into(),
            ))
        }
    })
}

fn binop_to_eqop(opts: &Options, op: &ast_defs::Bop) -> Option<EqOp> {
    use {ast_defs::Bop as B, EqOp::*};
    let check_int_overflow = opts
        .hhvm
        .hack_lang_flags
        .contains(LangFlags::CHECK_INT_OVERFLOW);
    match op {
        B::Plus => Some(if check_int_overflow {
            PlusEqualO
        } else {
            PlusEqual
        }),
        B::Minus => Some(if check_int_overflow {
            MinusEqualO
        } else {
            MinusEqual
        }),
        B::Star => Some(if check_int_overflow {
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
    let check_int_overflow = opts
        .hhvm
        .hack_lang_flags
        .contains(LangFlags::CHECK_INT_OVERFLOW);
    use ast_defs::Bop as B;
    Ok(match op {
        B::Plus => {
            if check_int_overflow {
                InstrSeq::make_addo()
            } else {
                InstrSeq::make_add()
            }
        }
        B::Minus => {
            if check_int_overflow {
                InstrSeq::make_subo()
            } else {
                InstrSeq::make_sub()
            }
        }
        B::Star => {
            if check_int_overflow {
                InstrSeq::make_mulo()
            } else {
                InstrSeq::make_mul()
            }
        }
        B::Slash => InstrSeq::make_div(),
        B::Eqeq => InstrSeq::make_eq(),
        B::Eqeqeq => InstrSeq::make_same(),
        B::Starstar => InstrSeq::make_pow(),
        B::Diff => InstrSeq::make_neq(),
        B::Diff2 => InstrSeq::make_nsame(),
        B::Lt => InstrSeq::make_lt(),
        B::Lte => InstrSeq::make_lte(),
        B::Gt => InstrSeq::make_gt(),
        B::Gte => InstrSeq::make_gte(),
        B::Dot => InstrSeq::make_concat(),
        B::Amp => InstrSeq::make_bitand(),
        B::Bar => InstrSeq::make_bitor(),
        B::Ltlt => InstrSeq::make_shl(),
        B::Gtgt => InstrSeq::make_shr(),
        B::Cmp => InstrSeq::make_cmp(),
        B::Percent => InstrSeq::make_mod(),
        B::Xor => InstrSeq::make_bitxor(),
        B::LogXor => InstrSeq::make_xor(),
        B::Eq(_) => return Err(Unrecoverable("assignment is emitted differently".into())),
        B::QuestionQuestion => {
            return Err(Unrecoverable(
                "null coalescence is emitted differently".into(),
            ))
        }
        B::Barbar | B::Ampamp => {
            return Err(Unrecoverable(
                "short-circuiting operator cannot be generated as a simple binop".into(),
            ))
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
                InstrSeq::make_cgetl2(get_local(e, env, &l.0, local_id::get_name(&l.1))?),
                true,
            )
        }
        _ => (emit_expr(e, env, expr)?, false),
    })
}

fn emit_two_exprs(
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
            vec![emit_pos(e, outer_pos)?, instrs2, instrs1]
        } else {
            vec![instrs2, emit_pos(e, outer_pos)?, instrs1]
        }
    } else if instrs2_is_var {
        vec![instrs1, emit_pos(e, outer_pos)?, instrs2]
    } else {
        vec![instrs1, instrs2, emit_pos(e, outer_pos)?]
    }))
}

fn emit_quiet_expr(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    expr: &tast::Expr,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq, Option<NumParams>)> {
    unimplemented!()
}

fn emit_null_coalesce_assignment(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    e1: &tast::Expr,
    e2: &tast::Expr,
) -> Result {
    unimplemented!()
}

fn emit_binop(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    (op, e1, e2): &(ast_defs::Bop, tast::Expr, tast::Expr),
) -> Result {
    use ast_defs::Bop as B;
    match op {
        B::Ampamp | B::Barbar => unimplemented!("TODO(hrust)"),
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
            Ok(InstrSeq::gather(vec![
                emit_quiet_expr(e, env, pos, e1, false)?.0,
                InstrSeq::make_dup(),
                InstrSeq::make_istypec(IstypeOp::OpNull),
                InstrSeq::make_not(),
                InstrSeq::make_jmpnz(end_label.clone()),
                InstrSeq::make_popc(),
                emit_expr(e, env, e2)?,
                InstrSeq::make_label(end_label),
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
                        InstrSeq::make_not(),
                    ])),
                    B::Diff2 if e1.1.is_null() => Ok(InstrSeq::gather(vec![
                        emit_is_null(e, env, e2)?,
                        InstrSeq::make_not(),
                    ])),
                    _ => default(e),
                }
            } else {
                default(e)
            }
        }
    }
}

fn emit_pipe(env: &Env, (_, e1, e2): &(aast_defs::Lid, tast::Expr, tast::Expr)) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_is_hint(env: &Env, pos: &Pos, h: &aast_defs::Hint) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_as(
    env: &Env,
    pos: &Pos,
    (e, h, is_nullable): &(tast::Expr, aast_defs::Hint, bool),
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_cast(env: &Env, pos: &Pos, (h, e): &(aast_defs::Hint, tast::Expr)) -> Result {
    unimplemented!("TODO(hrust)")
}

pub fn emit_unset_expr<Ex, Fb, En, Hi>(env: &Env, e: &aast::Expr<Ex, Fb, En, Hi>) -> Result {
    unimplemented!("TODO(hrust)")
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
        ([], _) => InstrSeq::make_int(-1),
        (_, false) => return raise_fatal("expects no more than 3 arguments"),
        (_, true) => return raise_fatal("expects no more than 4 arguments"),
    };

    let (base_expr, cls_expr, base_setup, base_stack, cls_stack) = emit_base(
        e,
        env,
        EmitBaseArgs {
            is_object: false,
            null_coalesce_assignment: None,
            base_offset: 3,
            rhs_stack_size: 3,
        },
        MemberOpMode::Define,
        base,
    )?;
    Ok(InstrSeq::gather(vec![
        base_expr,
        cls_expr,
        emit_expr(e, env, offset)?,
        emit_expr(e, env, src)?,
        count_instrs,
        base_setup,
        InstrSeq::make_instr(Instruct::IFinal(InstructFinal::SetRangeM(
            (base_stack + cls_stack)
                .try_into()
                .expect("StackIndex overflow"),
            kind.op,
            kind.size.try_into().expect("Setrange size overflow"),
        ))),
    ]))
}

pub fn is_reified_tparam(env: &Env, is_fun: bool, name: &str) -> Option<(usize, bool)> {
    let is = |tparams: &Vec<tast::Tparam>| {
        let is_soft = |ual: &Vec<tast::UserAttribute>| {
            ual.iter().any(|ua| &ua.name.1 == user_attributes::SOFT)
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
        env.scope.get_fun_tparams().and_then(is)
    } else {
        is(&env.scope.get_class_params().list)
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
    args: EmitBaseArgs,
    mode: MemberOpMode,
    ex: &tast::Expr,
) -> Result<(InstrSeq, InstrSeq, InstrSeq, StackIndex, StackIndex)> {
    let _notice = BareThisOp::Notice;
    unimplemented!("TODO(hrust)")
}

#[derive(Debug, Default)]
struct EmitBaseArgs {
    is_object: bool,
    null_coalesce_assignment: Option<bool>,
    base_offset: StackIndex,
    rhs_stack_size: StackIndex,
}

pub fn emit_ignored_expr(emitter: &mut Emitter, env: &Env, pos: &Pos, expr: &tast::Expr) -> Result {
    if let Some(es) = expr.1.as_expr_list() {
        Ok(InstrSeq::gather(
            es.iter()
                .map(|e| emit_ignored_expr(emitter, env, pos, e))
                .collect::<Result<Vec<_>>>()?,
        ))
    } else {
        Ok(InstrSeq::gather(vec![
            emit_expr(emitter, env, expr)?,
            emit_pos_then(emitter, pos, InstrSeq::make_popc())?,
        ]))
    }
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
                        Some(local.clone())
                    } else {
                        None
                    };
                    let (instr_lhs, instr_assign) =
                        emit_lval_op_list(e, env, pos, loc, &[], expr1, false)?;
                    Ok((
                        InstrSeq::gather(vec![
                            instr_lhs,
                            instr_rhs,
                            InstrSeq::make_popl(local.clone()),
                        ]),
                        instr_assign,
                        InstrSeq::make_pushl(local),
                    ))
                })
            }
        }
        _ => e.local_scope(|e| {
            let (rhs_instrs, rhs_stack_size) = match expr2 {
                None => (InstrSeq::Empty, 0),
                Some(tast::Expr(_, tast::Expr_::Yield(af))) => {
                    let temp = e.local_gen_mut().get_unnamed();
                    (
                        InstrSeq::gather(vec![
                            emit_yield(e, env, pos, af)?,
                            InstrSeq::make_setl(temp.clone()),
                            InstrSeq::make_popc(),
                            InstrSeq::make_pushl(temp),
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
            if ((c.1).1)
                .as_id()
                .map_or(false, |id| id.1 == special_functions::ECHO) =>
        {
            false
        }
        E_::Lvar(_)
        | E_::ArrayGet(_)
        | E_::ObjGet(_)
        | E_::ClassGet(_)
        | E_::PUAtom(_)
        | E_::Call(_)
        | E_::FunctionPointer(_)
        | E_::New(_)
        | E_::Record(_)
        | E_::ExprList(_)
        | E_::Yield(_)
        | E_::Cast(_)
        | E_::Eif(_)
        | E_::Array(_)
        | E_::Varray(_)
        | E_::Darray(_)
        | E_::Collection(_)
        | E_::Clone(_)
        | E_::Unop(_)
        | E_::As(_)
        | E_::Await(_) => true,
        E_::Pipe(p) => can_use_as_rhs_in_list_assignment(&(p.2).1)?,
        E_::Binop(b) if b.0.is_eq() => can_use_as_rhs_in_list_assignment(&(b.2).1)?,
        E_::Binop(b) => b.0.is_plus() || b.0.is_question_question() || b.0.is_any_eq(),
        E_::PUIdentifier(_) => {
            return Err(Unrecoverable(
                "TODO(T35357243): Pocket Universes syntax must be erased by now".into(),
            ))
        }
        _ => false,
    })
}

pub fn emit_lval_op_list(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    local: Option<local::Type>,
    indices: &[isize],
    expr: &tast::Expr,
    last_usage: bool,
) -> Result<(InstrSeq, InstrSeq)> {
    unimplemented!()
}

pub fn emit_lval_op_nonlist(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    op: LValOp,
    expr: &tast::Expr,
    rhs_instrs: InstrSeq,
    rhs_stack_size: usize,
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

pub fn emit_final_global_op(e: &mut Emitter, pos: &Pos, op: LValOp) -> Result {
    use LValOp as L;
    match op {
        L::Set => emit_pos_then(e, pos, InstrSeq::make_setg()),
        L::SetOp(op) => Ok(InstrSeq::make_setopg(op)),
        L::IncDec(op) => Ok(InstrSeq::make_incdecg(op)),
        L::Unset => emit_pos_then(e, pos, InstrSeq::make_unsetg()),
    }
}

pub fn emit_final_local_op(e: &mut Emitter, pos: &Pos, op: LValOp, lid: local::Type) -> Result {
    use LValOp as L;
    emit_pos_then(
        e,
        pos,
        match op {
            L::Set => InstrSeq::make_setl(lid),
            L::SetOp(op) => InstrSeq::make_setopl(lid, op),
            L::IncDec(op) => InstrSeq::make_incdecl(lid, op),
            L::Unset => InstrSeq::make_unsetl(lid),
        },
    )
}

pub fn emit_lval_op_nonlist_steps(
    e: &mut Emitter,
    env: &Env,
    outer_pos: &Pos,
    op: LValOp,
    expr: &tast::Expr,
    rhs_instrs: InstrSeq,
    rhs_stack_size: usize,
    null_coalesce_assignment: bool,
) -> Result<(InstrSeq, InstrSeq, InstrSeq)> {
    let f = |env: &mut Env| {
        use tast::Expr_ as E_;
        let pos = &expr.0;
        Ok(match &expr.1 {
            E_::Lvar(v) if superglobals::is_any_global(local_id::get_name(&v.1)) => (
                emit_pos_then(
                    e,
                    &v.0,
                    InstrSeq::make_string(string_utils::lstrip(local_id::get_name(&v.1), "$")),
                )?,
                rhs_instrs,
                emit_final_global_op(e, outer_pos, op)?,
            ),
            E_::Lvar(v) if is_local_this(env, &v.1) && op.is_incdec() => (
                emit_local(e, env, BareThisOp::Notice, v)?,
                rhs_instrs,
                InstrSeq::Empty,
            ),
            E_::Lvar(v) if !is_local_this(env, &v.1) || op == LValOp::Unset => {
                (InstrSeq::Empty, rhs_instrs, {
                    let lid = get_local(e, env, &v.0, &(v.1).1)?;
                    emit_final_local_op(e, outer_pos, op, lid)?
                })
            }
            E_::ArrayGet(_) => unimplemented!(),
            E_::ObjGet(_) => unimplemented!(),
            E_::ClassGet(_) => unimplemented!(),
            E_::Unop(uop) => (
                InstrSeq::Empty,
                rhs_instrs,
                InstrSeq::gather(vec![
                    emit_lval_op_nonlist(
                        e,
                        env,
                        pos,
                        op,
                        &uop.1,
                        InstrSeq::Empty,
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
                ))
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

pub fn emit_reified_arg(
    _emitter: &mut Emitter,
    _env: &Env,
    _pos: &Pos,
    isas: bool,
    hint: &tast::Hint,
) -> Result<(InstrSeq, bool)> {
    unimplemented!("TODO(hrust)")
}

pub fn get_local(e: &mut Emitter, env: &Env, pos: &Pos, s: &str) -> Result<local::Type> {
    if s == special_idents::DOLLAR_DOLLAR {
        unimplemented!()
    } else if special_idents::is_tmp_var(s) {
        Ok(e.local_gen().get_unnamed_for_tempname(s).clone())
    } else {
        Ok(local::Type::Named(s.into()))
    }
}

pub fn emit_is_null(e: &mut Emitter, env: &Env, expr: &tast::Expr) -> Result {
    if let Some(tast::Lid(pos, id)) = expr.1.as_lvar() {
        if !is_local_this(env, id) {
            return Ok(InstrSeq::make_istypel(
                get_local(e, env, pos, local_id::get_name(id))?,
                IstypeOp::OpNull,
            ));
        }
    }

    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        InstrSeq::make_istypec(IstypeOp::OpNull),
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
                        instrs: emit_pos_then(e, pos, InstrSeq::make_jmp(label.clone()))?,
                        is_fallthrough: false,
                        is_label_used: true,
                    }
                } else {
                    EmitJmpResult {
                        instrs: emit_pos_then(e, pos, InstrSeq::Empty)?,
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
                                    e,
                                    pos,
                                    InstrSeq::gather(vec![r1.instrs, r2.instrs]),
                                )?,
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
                                    e,
                                    pos,
                                    InstrSeq::gather(vec![
                                        r1.instrs,
                                        InstrSeq::optional(
                                            r1.is_label_used,
                                            vec![InstrSeq::make_label(skip_label)],
                                        ),
                                    ]),
                                )?,
                                is_fallthrough: r1.is_label_used,
                                is_label_used: false,
                            }
                        } else {
                            let r2 = emit_jmpnz(e, env, &bo.2, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    e,
                                    pos,
                                    InstrSeq::gather(vec![
                                        r1.instrs,
                                        r2.instrs,
                                        InstrSeq::optional(
                                            r1.is_label_used,
                                            vec![InstrSeq::make_label(skip_label)],
                                        ),
                                    ]),
                                )?,
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
                                e,
                                pos,
                                InstrSeq::gather(vec![
                                    is_null,
                                    InstrSeq::make_jmpnz(label.clone()),
                                ]),
                            )?,
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
                                e,
                                pos,
                                InstrSeq::gather(vec![is_null, InstrSeq::make_jmpz(label.clone())]),
                            )?,
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    _ => {
                        let instr = emit_expr(e, env, expr)?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                e,
                                pos,
                                InstrSeq::gather(vec![instr, InstrSeq::make_jmpnz(label.clone())]),
                            )?,
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
                        instrs: emit_pos_then(e, pos, InstrSeq::Empty)?,
                        is_fallthrough: true,
                        is_label_used: false,
                    }
                } else {
                    EmitJmpResult {
                        instrs: emit_pos_then(e, pos, InstrSeq::make_jmp(label.clone()))?,
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
                                    e,
                                    pos,
                                    InstrSeq::gather(vec![
                                        r1.instrs,
                                        InstrSeq::optional(
                                            r1.is_label_used,
                                            vec![InstrSeq::make_label(skip_label)],
                                        ),
                                    ]),
                                )?,
                                is_fallthrough: r1.is_label_used,
                                is_label_used: false,
                            }
                        } else {
                            let r2 = emit_jmpz(e, env, &bo.2, label)?;
                            EmitJmpResult {
                                instrs: emit_pos_then(
                                    e,
                                    pos,
                                    InstrSeq::gather(vec![
                                        r1.instrs,
                                        r2.instrs,
                                        InstrSeq::optional(
                                            r1.is_label_used,
                                            vec![InstrSeq::make_label(skip_label)],
                                        ),
                                    ]),
                                )?,
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
                                    e,
                                    pos,
                                    InstrSeq::gather(vec![r1.instrs, r2.instrs]),
                                )?,
                                is_fallthrough: r2.is_fallthrough,
                                is_label_used: r1.is_label_used || r2.is_label_used,
                            }
                        } else {
                            EmitJmpResult {
                                instrs: emit_pos_then(e, pos, r1.instrs)?,
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
                                e,
                                pos,
                                InstrSeq::gather(vec![is_null, InstrSeq::make_jmpz(label.clone())]),
                            )?,
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
                                e,
                                pos,
                                InstrSeq::gather(vec![
                                    is_null,
                                    InstrSeq::make_jmpnz(label.clone()),
                                ]),
                            )?,
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                    _ => {
                        let instr = emit_expr(e, env, expr)?;
                        EmitJmpResult {
                            instrs: emit_pos_then(
                                e,
                                pos,
                                InstrSeq::gather(vec![instr, InstrSeq::make_jmpz(label.clone())]),
                            )?,
                            is_fallthrough: true,
                            is_label_used: true,
                        }
                    }
                }
            }
        },
    )
}

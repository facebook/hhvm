// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(unused_variables, dead_code)]

use ast_constant_folder_rust as ast_constant_folder;
use ast_scope_rust::Scope;
use emit_fatal_rust as emit_fatal;
use emit_type_constant_rust as emit_type_constant;
use env::{emitter::Emitter, Env};
use hhbc_ast_rust::*;
use hhbc_id_rust::{class, method, r#const, Id};
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::{Error::Unrecoverable, InstrSeq, Result};
use label_rust::Label;
use local_rust as local;
use naming_special_names_rust::{pseudo_consts, special_idents, superglobals};
use options::{HhvmFlags, Options};
use oxidized::{aast, aast_defs, ast as tast, ast_defs, local_id, pos::Pos};
use scope_rust::scope;

use std::{collections::BTreeMap, convert::TryInto};

pub struct EmitJmpResult {
    // generated instruction sequence
    instrs: InstrSeq,
    // does instruction sequence fall through
    is_fallthrough: bool,
    // was label associated with emit operation used
    is_label_used: bool,
}

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
    hint: aast::Hint,
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
                .unwrap();
            emit_pos_then(emitter, pos, InstrSeq::make_typedvalue(v))
        }
        Expr_::PrefixedString(e) => emit_expr(emitter, env, &(&**e).1),
        Expr_::ParenthesizedExpr(e) => emit_expr(emitter, env, &*e),
        Expr_::Lvar(e) => {
            let Lid(pos, _) = &**e;
            Ok(InstrSeq::gather(vec![
                emit_pos(emitter, pos)?,
                emit_local(env, BareThisOp::Notice, &**e)?,
            ]))
        }
        Expr_::ClassConst(e) => emit_class_const(env, pos, &**e),
        Expr_::Unop(e) => emit_unop(env, pos, &**e),
        Expr_::Binop(e) => emit_binop(env, pos, &**e),
        Expr_::Pipe(e) => emit_pipe(env, &**e),
        Expr_::Is(is_expr) => {
            let (e, h) = &**is_expr;
            Ok(InstrSeq::gather(vec![
                emit_expr(emitter, env, e)?,
                emit_is_hint(env, pos, h)?,
            ]))
        }
        Expr_::As(e) => emit_as(env, pos, &**e),
        Expr_::Cast(e) => emit_cast(env, pos, &**e),
        Expr_::Eif(e) => emit_conditional_expr(env, pos, &**e),
        Expr_::ExprList(es) => Ok(InstrSeq::gather(
            es.iter()
                .map(|e| emit_expr(emitter, env, e).unwrap_or_default())
                .collect(),
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
                _ => emit_array_get(env, pos, QueryOp::CGet, &**e),
            }
        }
        Expr_::ObjGet(e) => emit_obj_get(env, pos, QueryOp::CGet, &**e),
        Expr_::Call(_) => emit_call_expr(env, pos, None, expr.as_call().unwrap()),
        Expr_::New(e) => emit_new(env, pos, &**e),
        Expr_::Record(e) => emit_record(env, pos, &**e),
        Expr_::Array(es) => emit_pos_then(emitter, pos, emit_collection(env, expression, es)?),
        Expr_::Darray(e) => {
            let instrs = emit_collection(env, expression, &mk_afkvalues(&(&**e).1))?;
            emit_pos_then(emitter, pos, instrs)
        }
        Expr_::Varray(e) => {
            let instrs = emit_collection(env, expression, &mk_afvalues(&(&**e).1))?;
            emit_pos_then(emitter, pos, instrs)
        }
        Expr_::Collection(e) => emit_named_collection_str(env, expression, &**e),
        Expr_::ValCollection(e) => {
            let (kind, _, es) = &**e;
            let fields = mk_afvalues(es);
            let collection_typ = match kind {
                aast_defs::VcKind::Vector => CollectionType::Vector,
                aast_defs::VcKind::ImmVector => CollectionType::ImmVector,
                aast_defs::VcKind::Set => CollectionType::Set,
                aast_defs::VcKind::ImmSet => CollectionType::ImmSet,
                _ => return emit_collection(env, expression, &fields),
            };
            emit_named_collection(env, pos, expression, &fields, collection_typ)
        }
        Expr_::Pair(e) => {
            let (e1, e2) = (**e).to_owned();
            let fields = mk_afvalues(&vec![e1, e2]);
            emit_named_collection(env, pos, expression, &fields, CollectionType::Pair)
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
                _ => return emit_collection(env, expression, &fields),
            };
            emit_named_collection(env, pos, expression, &fields, collection_typ)
        }
        Expr_::Clone(e) => emit_pos_then(emitter, pos, emit_clone(env, &**e)?),
        Expr_::Shape(e) => emit_pos_then(emitter, pos, emit_shape(env, expression, e)?),
        Expr_::Await(e) => emit_await(emitter, env, pos, &**e),
        Expr_::Yield(e) => emit_yield(env, pos, &**e),
        Expr_::Efun(e) => emit_pos_then(emitter, pos, emit_lambda(env, &**e)?),
        Expr_::ClassGet(e) => emit_class_get(env, QueryOp::CGet, &**e),
        Expr_::String2(es) => emit_string2(env, pos, es),
        Expr_::BracedExpr(e) => emit_expr(emitter, env, &**e),
        Expr_::Id(e) => {
            let instrs = emit_id(emitter, env, &**e)?;
            emit_pos_then(emitter, pos, instrs)
        }
        Expr_::Xml(e) => emit_xhp(env, pos, &**e),
        Expr_::Callconv(e) => emit_callconv(&**e),
        Expr_::Import(e) => emit_import(env, pos, &**e),
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
            emit_fatal::raise_fatal_parse(
                p,
                "error converting end of line from usize to isize".to_string(),
            )
        })?),
        G__NAMESPACE__ => InstrSeq::make_string(env.namespace.name.as_ref().map_or("", |s| &s[..])),
        EXIT | DIE => return emit_exit(emitter, env, None),
        _ => {
            panic!("TODO: uncomment after D19350786 lands")
            // let cid: ConstId = r#const::Type::from_ast_name(&s);
            // emit_symbol_refs::State::add_constant(emitter, cid.clone());
            // return emit_pos_then(emitter, p, InstrSeq::make_lit_const(CnsE(cid)));
        }
    };
    Ok(res)
}

fn emit_exit(emitter: &mut Emitter, env: &Env, expr_opt: Option<&tast::Expr>) -> Result {
    Ok(InstrSeq::gather(vec![
        expr_opt.map_or_else(
            || InstrSeq::make_int(0),
            |e| emit_expr(emitter, env, e).unwrap_or_default(),
        ),
        InstrSeq::make_exit(),
    ]))
}

fn emit_callconv((kind, _): &(ast_defs::ParamKind, tast::Expr)) -> Result {
    if let ast_defs::ParamKind::Pinout = kind {
        panic!("emit_callconv: This should have been caught at emit_arg")
    };
    unimplemented!("TODO(hrust)")
}

fn emit_xhp(
    env: &Env,
    pos: &Pos,
    (id, attributes, children): &(tast::Sid, Vec<tast::XhpAttribute>, Vec<tast::Expr>),
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_yield(env: &Env, pos: &Pos, af: &tast::Afield) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_import(env: &Env, pos: &Pos, (flavor, expr): &(tast::ImportFlavor, tast::Expr)) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_string2(env: &Env, pos: &Pos, es: &Vec<tast::Expr>) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_clone(env: &Env, expr: &tast::Expr) -> Result {
    unimplemented!("TODO(hrust)")
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
            let instrs = match e.as_call() {
                Some(call_expr) => emit_call_expr(env, pos, Some(after_await.clone()), call_expr)?,
                None => emit_expr(emitter, env, expr)?,
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

    Ok(scope::with_unnamed_local(emitter, |arr_local| {
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
                FcallArgs::new(None, None, None, Some(async_eager_label.clone()), 1),
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
                //TODO(milliechen) this needs to take in &mut Emitter
                // which isn't allowed while emitter's already being mutably borrowed
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
            ),
        ]);

        let after = InstrSeq::make_pushl(arr_local);

        (before, inner, after)
    }))
}

fn emit_iter<F: FnOnce(local::Type, local::Type) -> InstrSeq>(
    collection: InstrSeq,
    f: F,
) -> InstrSeq {
    unimplemented!("")
}

fn emit_shape(
    env: &Env,
    expr: &tast::Expr,
    fl: &Vec<(ast_defs::ShapeFieldName, tast::Expr)>,
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_named_collection(
    env: &Env,
    pos: &Pos,
    expr: &tast::Expr,
    fields: &Vec<tast::Afield>,
    collection_typ: CollectionType,
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_named_collection_str(
    env: &Env,
    expr: &tast::Expr,
    (ast_defs::Id(pos, name), _, fields): &(
        tast::Sid,
        Option<tast::CollectionTarg>,
        Vec<tast::Afield>,
    ),
) -> Result {
    unimplemented!("TODO(hrust)")
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

fn emit_collection(env: &Env, expression: &tast::Expr, fields: &Vec<tast::Afield>) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_record(
    env: &Env,
    pos: &Pos,
    (cid, is_array, es): &(tast::Sid, bool, Vec<(tast::Expr, tast::Expr)>),
) -> Result {
    let es = mk_afkvalues(es);
    unimplemented!("TODO(hrust)")
}

fn emit_call_expr(
    env: &Env,
    pos: &Pos,
    async_eager_label: Option<Label>,
    (_, e, targs, args, uarg): (
        &tast::CallType,
        &tast::Expr,
        &Vec<tast::Targ>,
        &Vec<tast::Expr>,
        &Option<tast::Expr>,
    ),
) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_new(
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
    unimplemented!("TODO(hrust)")
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

fn emit_local(env: &Env, notice: BareThisOp, lid: &aast_defs::Lid) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_class_const(env: &Env, pos: &Pos, (ci, id): &(tast::ClassId, ast_defs::Pstring)) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_unop(env: &Env, pos: &Pos, (uop, e): &(ast_defs::Uop, tast::Expr)) -> Result {
    unimplemented!("TODO(hrust)")
}

fn emit_binop(
    env: &Env,
    pos: &Pos,
    (op, e1, e2): &(ast_defs::Bop, tast::Expr, tast::Expr),
) -> Result {
    unimplemented!("TODO(hrust)")
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
    args: &[tast::Expr],
    last_arg: Option<&tast::Expr>,
) -> Result {
    let raise_fatal = |msg: &str| {
        Err(emit_fatal::raise_fatal_parse(
            pos,
            format!("{} {}", name, msg),
        ))
    };

    // NOTE(hrust) last_arg is separated because the caller
    // would otherwise need to clone both Vec<&Expr> and Expr,
    // or it would need to pass chained FixedSizeIterators
    let n = args.len();
    let (base, offset, src, n) = match last_arg {
        Some(a) if n >= 2 => (a, &args[n - 1], &args[n - 2], n - 2),
        None if n >= 3 => (&args[n - 1], &args[n - 2], &args[n - 3], n - 3),
        _ => return raise_fatal("expects at least 3 arguments"),
    };
    let count_instrs = match args.get(n - 1) {
        Some(c) if kind.vec => emit_expr(e, env, c)?,
        None => InstrSeq::make_int(-1),
        _ => {
            return if !kind.vec {
                raise_fatal("expects no more than 3 arguments")
            } else {
                raise_fatal("expects no more than 4 arguments")
            }
        }
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

pub fn emit_ignored_expr(
    _emitter: &mut Emitter,
    _env: &Env,
    _pos: &Pos,
    _expr: &tast::Expr,
) -> Result {
    unimplemented!("TODO(hrust)")
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused_variables)]
use ast_constant_folder_rust as ast_constant_folder;
use ast_scope_rust::Scope;
use emit_fatal_rust as emit_fatal;
use emit_type_constant_rust as emit_type_constant;
use env::{emitter::Emitter, Env};
use hhbc_ast_rust::*;
use instruction_sequence_rust::InstrSeq;
use label_rust::Label;
use local_rust as local;
use options::Options;

use naming_special_names_rust::{special_idents, superglobals};
use oxidized::{aast, aast_defs, ast as tast, ast_defs, local_id, pos::Pos};
use std::collections::BTreeMap;

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
    use std::{
        collections::{hash_map::Entry, HashMap},
        marker::PhantomData,
    };

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
        env: &'a Env,
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
    let _tv = emit_type_constant::hint_to_type_constant(opts, tparams, targ_map, hint);
    unimplemented!("TODO(hrust) after porting most of emit_adata")
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

pub fn emit_expr(
    emitter: &mut Emitter,
    env: &Env,
    expression: &tast::Expr,
) -> Result<InstrSeq, emit_fatal::Error> {
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
            let v = ast_constant_folder::expr_to_opt_typed_value(emitter, expression).unwrap();
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
        Expr_::Call(e) => emit_call_expr(env, pos, None, &**e),
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
        Expr_::Await(e) => emit_await(env, pos, &**e),
        Expr_::Yield(e) => emit_yield(env, pos, &**e),
        Expr_::Efun(e) => emit_pos_then(emitter, pos, emit_lambda(env, &**e)?),
        Expr_::ClassGet(e) => emit_class_get(env, QueryOp::CGet, &**e),
        Expr_::String2(es) => emit_string2(env, pos, es),
        Expr_::BracedExpr(e) => emit_expr(emitter, env, &**e),
        Expr_::Id(id) => emit_pos_then(emitter, pos, emit_id(env, &**id)?),
        Expr_::Xml(e) => emit_xhp(env, pos, &**e),
        Expr_::Callconv(e) => emit_callconv(env, &**e),
        Expr_::Import(e) => emit_import(env, pos, &**e),
        Expr_::Omitted => Ok(InstrSeq::Empty),
        Expr_::YieldBreak => panic!("yield break should be in statement position"),
        Expr_::YieldFrom(_) => panic!("complex yield_from expression"),
        Expr_::Lfun(_) => {
            panic!("expected Lfun to be converted to Efun during closure conversion emit_expr")
        }
        _ => unimplemented!("TODO(hrust)"),
    }
}

fn emit_pos_then(
    emitter: &Emitter,
    pos: &Pos,
    instrs: InstrSeq,
) -> Result<InstrSeq, emit_fatal::Error> {
    Ok(emit_pos_rust::emit_pos_then(emitter, pos, instrs))
}

fn emit_pos(emitter: &Emitter, pos: &Pos) -> Result<InstrSeq, emit_fatal::Error> {
    Ok(emit_pos_rust::emit_pos(emitter, pos))
}

fn emit_id(env: &Env, id: &tast::Sid) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_callconv(
    env: &Env,
    (kind, expr): &(ast_defs::ParamKind, tast::Expr),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_xhp(
    env: &Env,
    pos: &Pos,
    (id, attributes, children): &(tast::Sid, Vec<tast::XhpAttribute>, Vec<tast::Expr>),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_yield(env: &Env, pos: &Pos, af: &tast::Afield) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_import(
    env: &Env,
    pos: &Pos,
    (flavor, expr): &(tast::ImportFlavor, tast::Expr),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_string2(env: &Env, pos: &Pos, es: &Vec<tast::Expr>) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_clone(env: &Env, expr: &tast::Expr) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_lambda(
    env: &Env,
    (fndef, ids): &(tast::Fun_, Vec<aast_defs::Lid>),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_await(env: &Env, pos: &Pos, expr: &tast::Expr) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_shape(
    env: &Env,
    expr: &tast::Expr,
    fl: &Vec<(ast_defs::ShapeFieldName, tast::Expr)>,
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_named_collection(
    env: &Env,
    pos: &Pos,
    expr: &tast::Expr,
    fields: &Vec<tast::Afield>,
    collection_typ: CollectionType,
) -> Result<InstrSeq, emit_fatal::Error> {
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
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn mk_afkvalues(es: &Vec<(tast::Expr, tast::Expr)>) -> Vec<tast::Afield> {
    es.to_owned()
        .into_iter()
        .map(|(e1, e2)| tast::Afield::mk_afkvalue(e1, e2))
        .collect()
}

fn mk_afvalues(es: &Vec<(tast::Expr)>) -> Vec<tast::Afield> {
    es.to_owned()
        .into_iter()
        .map(|e| tast::Afield::mk_afvalue(e))
        .collect()
}

fn emit_collection(
    env: &Env,
    expression: &tast::Expr,
    fields: &Vec<tast::Afield>,
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_record(
    env: &Env,
    pos: &Pos,
    (cid, is_array, es): &(tast::Sid, bool, Vec<(tast::Expr, tast::Expr)>),
) -> Result<InstrSeq, emit_fatal::Error> {
    let es = mk_afkvalues(es);
    unimplemented!("TODO(hrust)")
}

fn emit_call_expr(
    env: &Env,
    pos: &Pos,
    async_eager_label: Option<Label>,
    (_, e, targs, args, uarg): &(
        tast::CallType,
        tast::Expr,
        Vec<tast::Targ>,
        Vec<tast::Expr>,
        Option<tast::Expr>,
    ),
) -> Result<InstrSeq, emit_fatal::Error> {
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
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_obj_get(
    env: &Env,
    pos: &Pos,
    query_op: QueryOp,
    (expr, prop, nullflavor): &(tast::Expr, tast::Expr, ast_defs::OgNullFlavor),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_array_get(
    env: &Env,
    pos: &Pos,
    query_op: QueryOp,
    (base_expr, opt_elem_expr): &(tast::Expr, Option<tast::Expr>),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_class_get(
    env: &Env,
    query_op: QueryOp,
    (cid, cls_get_expr): &(tast::ClassId, tast::ClassGetExpr),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_conditional_expr(
    env: &Env,
    pos: &Pos,
    (etest, etrue, efalse): &(tast::Expr, Option<tast::Expr>, tast::Expr),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_local(
    env: &Env,
    notice: BareThisOp,
    lid: &aast_defs::Lid,
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_class_const(
    env: &Env,
    pos: &Pos,
    (ci, id): &(tast::ClassId, ast_defs::Pstring),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_unop(
    env: &Env,
    pos: &Pos,
    (uop, e): &(ast_defs::Uop, tast::Expr),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_binop(
    env: &Env,
    pos: &Pos,
    (op, e1, e2): &(ast_defs::Bop, tast::Expr, tast::Expr),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_pipe(
    env: &Env,
    (_, e1, e2): &(aast_defs::Lid, tast::Expr, tast::Expr),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_is_hint(env: &Env, pos: &Pos, h: &aast_defs::Hint) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_as(
    env: &Env,
    pos: &Pos,
    (e, h, is_nullable): &(tast::Expr, aast_defs::Hint, bool),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

fn emit_cast(
    env: &Env,
    pos: &Pos,
    (h, e): &(aast_defs::Hint, tast::Expr),
) -> Result<InstrSeq, emit_fatal::Error> {
    unimplemented!("TODO(hrust)")
}

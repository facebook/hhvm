#![allow(unused)]
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use hash::HashSet;
use ir::instr::HasLoc;
use ir::instr::Hhbc;
use ir::instr::IncDecOp;
use ir::instr::Predicate;
use ir::instr::Special;
use ir::instr::Terminator;
use ir::instr::Textual;
use ir::instr::TextualHackBuiltinParam;
use ir::Block;
use ir::BlockId;
use ir::Constant;
use ir::ConstantId;
use ir::Func;
use ir::Instr;
use ir::InstrId;
use ir::LocId;
use ir::LocalId;
use ir::SrcLoc;
use ir::StringInterner;
use ir::TryCatchId;
use ir::ValueId;
use itertools::Itertools;
use log::trace;

use crate::hack;
use crate::mangle::Mangle;
use crate::mangle::MangleId;
use crate::state::UnitState;
use crate::textual;
use crate::textual::Sid;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// Functions are defined as taking a param bundle.
///
/// f(params: HackParams): mixed;
pub(crate) fn write_function(
    w: &mut dyn std::io::Write,
    unit_state: &mut UnitState<'_>,
    function: &ir::Function<'_>,
) -> Result {
    trace!("Convert Function {}", function.name.as_bstr());

    textual::write_function(
        w,
        &unit_state.unit.strings,
        &function.name.mangle(),
        function.func.loc(function.func.span),
        &[("params", tx_ty!(HackParams))],
        tx_ty!(mixed),
        |w| write_func(w, unit_state, &function.func),
    )?;

    Ok(())
}

fn write_func(
    w: &mut textual::FuncWriter<'_>,
    unit_state: &mut UnitState<'_>,
    func: &ir::Func<'_>,
) -> Result {
    let func = func.clone();
    let func = crate::lower::lower(func, &unit_state.unit.strings);
    let func = rewrite_prelude(func);
    let mut func = rewrite_jmp_ops(func);
    ir::passes::clean::run(&mut func);

    let mut state = FuncState::new(unit_state.unit, &func);

    for bid in func.block_ids() {
        write_block(w, &mut state, bid)?;
    }

    unit_state.external_funcs.extend(state.external_funcs);

    Ok(())
}

fn write_block(w: &mut textual::FuncWriter<'_>, state: &mut FuncState<'_>, bid: BlockId) -> Result {
    trace!("  Block {bid}");
    let block = state.func.block(bid);
    assert!(block.tcid == TryCatchId::None);

    let params = block
        .params
        .iter()
        .map(|iid| state.alloc_sid_for_iid(w, *iid))
        .collect_vec();
    w.write_label(bid, &params)?;

    for iid in block.iids() {
        write_instr(w, state, iid)?;
    }
    Ok(())
}

fn write_instr(w: &mut textual::FuncWriter<'_>, state: &mut FuncState<'_>, iid: InstrId) -> Result {
    let instr = state.func.instr(iid);
    trace!("    Instr {iid}: {instr:?}");

    state.update_loc(w, instr.loc_id())?;

    // In general don't write directly to `w` here - isolate the formatting to
    // the `textual` crate.

    match *instr {
        Instr::Call(ref call) => write_call(w, state, iid, call)?,
        Instr::Hhbc(Hhbc::CGetL(lid, _)) => write_load_var(w, state, iid, lid)?,
        Instr::Hhbc(Hhbc::IncDecL(lid, op, _)) => write_inc_dec_l(w, state, iid, lid, op)?,
        Instr::Hhbc(Hhbc::SetL(vid, lid, _)) => write_set_var(w, state, lid, vid)?,
        Instr::MemberOp(ref mop) => crate::member_op::write(w, state, iid, mop)?,
        Instr::Special(Special::Textual(Textual::AssertFalse(vid, _))) => {
            // I think "prune_not" means "stop if this expression IS true"...
            w.prune_not(textual::Expr::call("hack_is_true", [state.lookup_vid(vid)]))?;
        }
        Instr::Special(Special::Textual(Textual::AssertTrue(vid, _))) => {
            // I think "prune" means "stop if this expression IS NOT true"...
            w.prune(textual::Expr::call("hack_is_true", [state.lookup_vid(vid)]))?;
        }
        Instr::Special(Special::Textual(Textual::HackBuiltin {
            ref target,
            ref params,
            ref values,
            loc: _,
        })) => write_builtin(w, state, iid, target, params, values)?,
        Instr::Terminator(Terminator::Enter(bid, _) | Terminator::Jmp(bid, _)) => {
            w.jmp(&[bid], ())?;
        }
        Instr::Terminator(Terminator::JmpArgs(bid, ref params, _)) => {
            w.jmp(
                &[bid],
                params.iter().map(|v| state.lookup_vid(*v)).collect_vec(),
            )?;
        }
        Instr::Terminator(Terminator::JmpOp {
            cond: _,
            pred: _,
            targets: [true_bid, false_bid],
            loc: _,
        }) => {
            // We just need to emit the jmp - the rewrite_jmp_ops() pass should
            // have already inserted assert in place on the target bids.
            w.jmp(&[true_bid, false_bid], ())?;
        }
        Instr::Terminator(Terminator::Ret(vid, _)) => {
            w.ret(state.lookup_vid(vid))?;
        }
        _ => {
            // This should only handle instructions that can't be rewritten into
            // a simpler form (like control flow and generic calls). Everything
            // else should be handled in lower().
            todo!("unhandled instr: {instr:?}");
        }
    }

    Ok(())
}

fn write_builtin(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
    target: &str,
    params: &[TextualHackBuiltinParam],
    values: &[ValueId],
) -> Result {
    let mut values = values.iter();
    let params = params
        .iter()
        .map(|param| match *param {
            TextualHackBuiltinParam::Null => textual::Expr::null(),
            TextualHackBuiltinParam::False => textual::Expr::false_(),
            TextualHackBuiltinParam::HackInt(i) => textual::Expr::hack_int(i),
            TextualHackBuiltinParam::HackString(ref s) => textual::Expr::hack_string(s.clone()),
            TextualHackBuiltinParam::Int(i) => textual::Expr::int(i),
            TextualHackBuiltinParam::String(ref s) => textual::Expr::string(s.clone()),
            TextualHackBuiltinParam::True => textual::Expr::true_(),
            TextualHackBuiltinParam::Value => {
                let vid = *values.next().unwrap();
                let sid = state.lookup_vid(vid);
                textual::Expr::Sid(sid)
            }
        })
        .collect_vec();

    let output = w.call(target, params)?;
    state.set_iid(iid, output);
    Ok(())
}

fn write_load_var(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
    lid: LocalId,
) -> Result {
    let sid = w.load(&textual::Ty::Mixed, textual::Expr::deref(lid))?;
    state.set_iid(iid, sid);
    Ok(())
}

fn write_set_var(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    lid: LocalId,
    vid: ValueId,
) -> Result {
    w.store(
        textual::Expr::deref(lid),
        state.lookup_vid(vid),
        &textual::Ty::Mixed,
    )
}

fn write_call(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
    call: &ir::Call,
) -> Result {
    use ir::instr::CallDetail;
    use ir::instr::FCallArgsFlags;

    let ir::Call {
        ref operands,
        context,
        ref detail,
        flags,
        num_rets,
        ref inouts,
        ref readonly,
        loc: _,
    } = *call;

    assert!(state.strings().lookup_bytes(context).is_empty());
    assert!(inouts.as_ref().map_or(true, |inouts| inouts.is_empty()));
    assert!(readonly.as_ref().map_or(true, |ro| ro.is_empty()));
    assert!(num_rets < 2);

    if flags & FCallArgsFlags::HasUnpack != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::HasGenerics != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::LockWhileUnwinding != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::SkipRepack != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::SkipCoeffectsCheck != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::EnforceMutableReturn != 0 {
        // todo!();
    }
    if flags & FCallArgsFlags::EnforceReadonlyThis != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::ExplicitContext != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::HasInOut != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::EnforceInOut != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::EnforceReadonly != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::HasAsyncEagerOffset != 0 {
        todo!();
    }
    if flags & FCallArgsFlags::NumArgsStart != 0 {
        todo!();
    }

    let output = match *detail {
        CallDetail::FCallClsMethod { .. } => todo!(),
        CallDetail::FCallClsMethodD { .. } => todo!(),
        CallDetail::FCallClsMethodM { .. } => todo!(),
        CallDetail::FCallClsMethodS { .. } => todo!(),
        CallDetail::FCallClsMethodSD { .. } => todo!(),
        CallDetail::FCallCtor => todo!(),
        CallDetail::FCallFunc => todo!(),
        CallDetail::FCallFuncD { func } => {
            let target = func.mangle(state.strings());
            state.external_funcs.insert(target.to_string());
            let args = detail
                .args(operands)
                .iter()
                .map(|vid| state.lookup_vid(*vid))
                .collect_vec();
            let arg_pack = hack::call_builtin(w, hack::Builtin::ArgPack(args.len()), args)?;
            w.call(&target, [arg_pack])?
        }
        CallDetail::FCallObjMethod { .. } => todo!(),
        CallDetail::FCallObjMethodD { .. } => todo!(),
    };
    state.set_iid(iid, output);
    Ok(())
}

fn write_inc_dec_l<'a>(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'a>,
    iid: InstrId,
    lid: LocalId,
    op: IncDecOp,
) -> Result {
    let builtin = match op {
        IncDecOp::PreInc => hack::Builtin::Add,
        IncDecOp::PostInc => hack::Builtin::Add,
        IncDecOp::PreDec => hack::Builtin::Sub,
        IncDecOp::PostDec => hack::Builtin::Sub,
        IncDecOp::PreIncO => hack::Builtin::AddO,
        IncDecOp::PostIncO => hack::Builtin::AddO,
        IncDecOp::PreDecO => hack::Builtin::SubO,
        IncDecOp::PostDecO => hack::Builtin::SubO,
        _ => unreachable!(),
    };

    let pre = w.load(&textual::Ty::Mixed, textual::Expr::deref(lid))?;
    let post = hack::call_builtin(w, builtin, (pre, textual::Expr::hack_int(1)))?;
    w.store(textual::Expr::deref(lid), post, &textual::Ty::Mixed)?;

    let sid = match op {
        IncDecOp::PreInc | IncDecOp::PreDec | IncDecOp::PreIncO | IncDecOp::PreDecO => pre,
        IncDecOp::PostInc | IncDecOp::PostDec | IncDecOp::PostIncO | IncDecOp::PostDecO => post,
        _ => unreachable!(),
    };
    state.set_iid(iid, sid);

    Ok(())
}

pub(crate) struct FuncState<'a> {
    external_funcs: HashSet<String>,
    func: &'a ir::Func<'a>,
    iid_mapping: ir::InstrIdMap<Sid>,
    unit: &'a ir::Unit<'a>,
}

impl<'a> FuncState<'a> {
    fn new(unit: &'a ir::Unit<'a>, func: &'a ir::Func<'a>) -> Self {
        Self {
            external_funcs: Default::default(),
            func,
            iid_mapping: Default::default(),
            unit,
        }
    }

    pub fn alloc_sid_for_iid(&mut self, w: &mut textual::FuncWriter<'_>, iid: InstrId) -> Sid {
        let sid = w.alloc_sid();
        self.set_iid(iid, sid);
        sid
    }

    pub fn lookup_vid(&self, vid: ValueId) -> Sid {
        let iid = vid.expect_instr("instr expected");
        *self.iid_mapping.get(&iid).unwrap()
    }

    pub(crate) fn set_iid(&mut self, iid: InstrId, sid: Sid) {
        let old = self.iid_mapping.insert(iid, sid);
        assert!(old.is_none());
    }

    pub(crate) fn strings(&self) -> &StringInterner {
        &self.unit.strings
    }

    pub(crate) fn update_loc(&mut self, w: &mut textual::FuncWriter<'_>, loc: LocId) -> Result {
        if loc != LocId::NONE {
            let new = &self.func.locs[loc];
            w.write_loc(new)?;
        }
        Ok(())
    }
}

/// Rewrite the function prelude:
/// - Convert constants into builtins.
fn rewrite_prelude<'a>(func: ir::Func<'a>) -> ir::Func<'a> {
    let mut builder = ir::FuncBuilder::with_func(func);

    let mut remap = ir::ValueIdMap::default();

    // Swap out the initial block so we can inject our entry code.
    let entry_bid = builder.func.alloc_bid(Block::default());
    builder.func.blocks.swap(Func::ENTRY_BID, entry_bid);
    builder
        .func
        .remap_bids(&[(Func::ENTRY_BID, entry_bid)].into_iter().collect());

    builder.start_block(Func::ENTRY_BID);
    write_constants(&mut remap, &mut builder);
    builder.emit(Instr::jmp(entry_bid, LocId::NONE));

    let mut func = builder.finish();
    func.remap_vids(&remap);
    func
}

fn write_constants(remap: &mut ir::ValueIdMap<ValueId>, builder: &mut ir::FuncBuilder<'_>) {
    let constants = std::mem::take(&mut builder.func.constants);

    for (lid, constant) in constants.into_iter().enumerate() {
        let lid = ConstantId::from_usize(lid);
        trace!("    Const {lid}: {constant:?}");
        let src = ValueId::from_constant(lid);
        let loc = LocId::NONE;
        let vid = match constant {
            Constant::Bool(value) => {
                let params = vec![if value {
                    TextualHackBuiltinParam::True
                } else {
                    TextualHackBuiltinParam::False
                }];
                builder.emit(hack::builtin_instr(
                    hack::Builtin::Bool,
                    params,
                    vec![],
                    loc,
                ))
            }
            Constant::Dict(..) => todo!(),
            Constant::Dir => todo!(),
            Constant::Double(..) => todo!(),
            Constant::File => todo!(),
            Constant::FuncCred => todo!(),
            Constant::Int(i) => {
                let params = vec![TextualHackBuiltinParam::Int(i)];
                builder.emit(hack::builtin_instr(hack::Builtin::Int, params, vec![], loc))
            }
            Constant::Keyset(..) => todo!(),
            Constant::Method => todo!(),
            Constant::Named(..) => todo!(),
            Constant::NewCol(..) => todo!(),
            Constant::Null | Constant::Uninit => builder.emit(hack::builtin_instr(
                hack::Builtin::Null,
                vec![],
                vec![],
                loc,
            )),
            Constant::String(s) => {
                let params = vec![TextualHackBuiltinParam::HackString(s.to_vec())];
                builder.emit(hack::builtin_instr(
                    hack::Builtin::Copy,
                    params,
                    vec![],
                    loc,
                ))
            }
            Constant::Vec(..) => todo!(),
        };
        remap.insert(src, vid);
    }
}

/// Convert from a deterministic jump model to a non-deterministic model.
///
/// In Textual instead of "jump if" you say "jump to a, b" and then in 'a' and 'b'
/// you say "stop if my condition isn't met".
///
/// This inserts the needed 'assert_true' and 'assert_false' statements but
/// leaves the original JmpOp as a marker for where to jump to.
fn rewrite_jmp_ops<'a>(mut func: ir::Func<'a>) -> ir::Func<'a> {
    for bid in func.block_ids() {
        match *func.terminator(bid) {
            Terminator::JmpOp {
                cond,
                pred,
                targets: [mut true_bid, mut false_bid],
                loc,
            } => {
                // We need to rewrite this jump. Because we don't allow critical
                // edges we can just insert the 'assert' at the start of the
                // target block since we must be the only caller.
                trace!("    JmpOp at {bid} needs to be rewritten");
                match pred {
                    Predicate::Zero => {
                        std::mem::swap(&mut true_bid, &mut false_bid);
                    }
                    Predicate::NonZero => {}
                }

                let iid = func.alloc_instr(Instr::Special(Special::Textual(Textual::AssertTrue(
                    cond, loc,
                ))));
                func.block_mut(true_bid).iids.insert(0, iid);

                let iid = func.alloc_instr(Instr::Special(Special::Textual(Textual::AssertFalse(
                    cond, loc,
                ))));
                func.block_mut(false_bid).iids.insert(0, iid);
            }
            _ => {}
        }
    }

    func
}

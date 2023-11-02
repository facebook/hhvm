// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::fmt::Display;
use std::sync::Arc;

use hhbc::Instruct;
use hhbc::Opcode;
use hhbc::Pseudo;
use ir::instr;
use ir::instr::CmpOp;
use ir::instr::MemberKey;
use ir::instr::Terminator;
use ir::print::FmtLocId;
use ir::print::FmtRawBid;
use ir::print::FmtRawVid;
use ir::print::FmtSep;
use ir::FCallArgsFlags;
use ir::FunctionId;
use ir::Instr;
use ir::LocalId;
use ir::TryCatchId;
use ir::UnitBytesId;
use ir::ValueId;
use log::trace;

use crate::context::Addr;
use crate::context::Context;
use crate::context::MemberOpBuilder;
use crate::sequence::SequenceKind;

/// Convert the sequence that starts with the offset `addr`.
///
/// HHBC is a stack-based VM. We convert a sequence by unspilling the stack (see
/// below) and then emulate instruction by instruction. On a change of
/// control-flow we spill the stack and then record the target(s) as work to be
/// performed. We also record how many entries were on the caller's stack so the
/// target knows how many stack slots to expect as input. For instructions which
/// have optional control-flow (like a JmpZ) we unspill the stack immediately
/// and continue.
///
/// Spilling the stack consists of storing the stack slots to temporary VarId
/// variables (using Instr::Special(Special::Ssa(Ssa::SetVar))) which are
/// named with the stack depth (so the top-of-stack goes to VarId #0, the
/// next value on the stack goes to VarId #1, etc).
///
/// Unspilling just reverses that process by loading the stack slots (using
/// Instr::Special(Special::Ssa(Ssa::GetVar))) with the values of those VarIds.
///
/// The SSA pass is responsible for converting the SetVar and GetVar
/// instructions into actual SSA form.
///
pub(crate) fn convert_sequence<'a, 'b>(ctx: &mut Context<'a, 'b>, addr: Addr) {
    assert_eq!(ctx.stack.len(), 0);
    let seq = ctx.addr_to_seq[&addr].clone();

    ctx.builder.start_block(seq.bid);
    ctx.loc = seq.loc_id;

    trace!(
        "Sequence {:?}, bid = {}, loc = {}",
        seq.range,
        FmtRawBid(seq.bid),
        FmtLocId(&ctx.builder.func, ctx.loc)
    );

    add_catch_work(ctx, seq.tcid);

    match seq.kind {
        SequenceKind::Normal => {
            // If we never set a input_stack_size then nobody calls this
            // sequence directly - it must be a root (like the entry block or a
            // default parameter) and should expect no inputs.
            ctx.unspill_stack(seq.input_stack_size.unwrap_or(0));
        }
        SequenceKind::Catch => {
            // Expect a single value.
            ctx.alloc_and_push_param();
        }
    }

    for idx in Addr::range_to_iter(seq.range) {
        let instr = &ctx.instrs[idx.as_usize()];

        match instr {
            Instruct::Opcode(op) => {
                trace!(
                    "  [{idx}] {bid},%{iid} stack: [{stack}] instr {instr:?}, loc {loc:?}",
                    bid = FmtRawBid(ctx.builder.cur_bid()),
                    iid = ctx.builder.func.instrs_len(),
                    stack = FmtSep::comma(ctx.debug_get_stack().iter(), |w, vid| FmtRawVid(*vid)
                        .fmt(w)),
                    loc = ctx.loc
                );

                if !convert_opcode(ctx, op) {
                    // This opcode finishes the sequence.
                    break;
                }
            }
            Instruct::Pseudo(Pseudo::SrcLoc(src_loc)) => {
                ctx.loc = crate::context::add_loc(&mut ctx.builder, ctx.filename, src_loc);
            }
            Instruct::Pseudo(
                Pseudo::Label(_)
                | Pseudo::TryCatchBegin
                | Pseudo::TryCatchMiddle
                | Pseudo::TryCatchEnd,
            ) => {
                // We should never see these - they should be omitted by the
                // sequence gathering code.
                unreachable!();
            }
            Instruct::Pseudo(
                Pseudo::Break | Pseudo::Comment(..) | Pseudo::Continue | Pseudo::TypedValue(..),
            ) => {
                // We should never see these in an HHBC sequence outside of the
                // HackC emitter.
                unreachable!();
            }
        }
    }

    if !ctx.builder.func.is_terminated(ctx.builder.cur_bid()) {
        trace!("  Non-terminated sequence. Next = {:?}", seq.next);

        // This sequence ended on a non-terminal. That probably means it falls
        // into the next sequence. Pretend it just jumps to the next addr.
        let stack_size = ctx.spill_stack();

        if let Some(next) = seq.next {
            let bid = ctx.target_from_addr(next, stack_size);
            ctx.emit(Instr::Terminator(Terminator::Jmp(bid, ctx.loc)));
        } else {
            ctx.emit(Instr::unreachable());
        }
    }

    assert_eq!(ctx.stack.len(), 0, "Sequence ended with a non-empty stack!");
}

#[allow(clippy::todo)]
fn convert_base<'a, 'b>(ctx: &mut Context<'a, 'b>, base: &Opcode<'a>) {
    if let Some(mop) = ctx.member_op.as_ref() {
        panic!(
            "Unable to convert base {:?} with existing base {:?}",
            base, mop.base_op
        );
    }

    let mut operands: Vec<ValueId> = Vec::new();
    let mut locals: Vec<LocalId> = Vec::new();

    let loc = ctx.loc;
    let base_op = match *base {
        Opcode::BaseC(idx, mode) => {
            let vid = ctx.stack_get(idx as usize);
            operands.push(vid);
            instr::BaseOp::BaseC { mode, loc }
        }
        Opcode::BaseGC(idx, mode) => {
            let vid = ctx.stack_get(idx as usize);
            operands.push(vid);
            instr::BaseOp::BaseGC { mode, loc }
        }
        Opcode::BaseGL(..) => todo!(),
        Opcode::BaseSC(idx0, idx1, mode, readonly) => {
            let vid0 = ctx.stack_get(idx0 as usize);
            operands.push(vid0);
            let vid1 = ctx.stack_get(idx1 as usize);
            operands.push(vid1);
            instr::BaseOp::BaseSC {
                mode,
                readonly,
                loc,
            }
        }
        Opcode::BaseL(ref local, mode, readonly) => {
            let lid = convert_local(ctx, local);
            locals.push(lid);
            instr::BaseOp::BaseL {
                mode,
                readonly,
                loc,
            }
        }
        Opcode::BaseH => instr::BaseOp::BaseH { loc },
        _ => unreachable!(),
    };

    ctx.member_op = Some(MemberOpBuilder {
        operands,
        locals,
        base_op,
        intermediate_ops: Vec::new(),
    });
}

fn collect_args<'a, 'b>(ctx: &mut Context<'a, 'b>, num_args: u32) -> Vec<ValueId> {
    let mut res: Vec<ValueId> = (0..num_args).map(|_| ctx.pop()).collect();
    res.reverse();
    res
}

fn convert_call<'a, 'b>(ctx: &mut Context<'a, 'b>, call: &Opcode<'a>) {
    let fcall_args = match call {
        Opcode::FCallClsMethod(fcall_args, ..)
        | Opcode::FCallClsMethodD(fcall_args, ..)
        | Opcode::FCallClsMethodM(fcall_args, ..)
        | Opcode::FCallClsMethodS(fcall_args, ..)
        | Opcode::FCallClsMethodSD(fcall_args, ..)
        | Opcode::FCallCtor(fcall_args, ..)
        | Opcode::FCallFunc(fcall_args)
        | Opcode::FCallFuncD(fcall_args, ..)
        | Opcode::FCallObjMethod(fcall_args, ..)
        | Opcode::FCallObjMethodD(fcall_args, ..) => fcall_args,
        _ => unreachable!(),
    };

    let context = ctx.intern_ffi_str(fcall_args.context);

    let mut num_args = fcall_args.num_args;
    // These first two stack entries correspond to an HHVM ActRec (in
    // TypedValue-sized chunks - see kNumActRecCells).  The first entry fits
    // into the ActRec::m_thisUnsafe or ActRec::m_clsUnsafe values (they're a
    // union).  The second entry doesn't matter - it's filled in by the FCall
    // handler - but HackC always sets it to uninit.

    // implied obj parameter
    num_args += 1;
    // uninit
    num_args += 1;

    num_args += fcall_args.flags.contains(FCallArgsFlags::HasUnpack) as u32;
    num_args += fcall_args.flags.contains(FCallArgsFlags::HasGenerics) as u32;

    use instr::CallDetail;

    let detail = match *call {
        Opcode::FCallClsMethod(_, _, log) => {
            num_args += 2;
            CallDetail::FCallClsMethod { log }
        }
        Opcode::FCallClsMethodD(_, class, method) => {
            let clsid = ir::ClassId::from_hhbc(class, ctx.strings);
            let method = ir::MethodId::from_hhbc(method, ctx.strings);
            CallDetail::FCallClsMethodD { clsid, method }
        }
        Opcode::FCallClsMethodM(_, _, log, method) => {
            num_args += 1;
            let method = ir::MethodId::from_hhbc(method, ctx.strings);
            CallDetail::FCallClsMethodM { method, log }
        }
        Opcode::FCallClsMethodS(_, _, clsref) => {
            num_args += 1;
            CallDetail::FCallClsMethodS { clsref }
        }
        Opcode::FCallClsMethodSD(_, _, clsref, method) => {
            let method = ir::MethodId::from_hhbc(method, ctx.strings);
            CallDetail::FCallClsMethodSD { clsref, method }
        }
        Opcode::FCallCtor(_, _) => CallDetail::FCallCtor,
        Opcode::FCallFunc(_) => {
            num_args += 1;
            CallDetail::FCallFunc
        }
        Opcode::FCallFuncD(_, func) => {
            let func = FunctionId::from_hhbc(func, ctx.strings);
            CallDetail::FCallFuncD { func }
        }
        Opcode::FCallObjMethod(_, _, flavor) => {
            num_args += 1;
            CallDetail::FCallObjMethod { flavor }
        }
        Opcode::FCallObjMethodD(_, _, flavor, method) => {
            let method = ir::MethodId::from_hhbc(method, ctx.strings);
            CallDetail::FCallObjMethodD { flavor, method }
        }
        _ => unreachable!(),
    };

    let mut operands = collect_args(ctx, num_args);

    match *call {
        Opcode::FCallClsMethod(..)
        | Opcode::FCallClsMethodD(..)
        | Opcode::FCallClsMethodM(..)
        | Opcode::FCallClsMethodS(..)
        | Opcode::FCallClsMethodSD(..)
        | Opcode::FCallFunc(..)
        | Opcode::FCallFuncD(..) => {
            // Remove the two required uninit values.
            operands.splice(0..2, []);
            // We'd like to check that the removed items were actually uninit -
            // but because our stack could be coming from an unspill they could
            // be hidden behind block params.
        }
        Opcode::FCallCtor(..) | Opcode::FCallObjMethod(..) | Opcode::FCallObjMethodD(..) => {
            // Remove the required uninit value.
            operands.splice(1..2, []);
        }
        _ => unreachable!(),
    };

    // inout return slots
    let inouts: Option<Box<[u32]>> = {
        let mut buf: Vec<u32> = Vec::new();

        for (idx, &inout) in fcall_args.inouts.as_ref().iter().enumerate() {
            if inout {
                buf.push(idx as u32);
                ctx.pop();
            }
        }

        if buf.is_empty() {
            None
        } else {
            Some(buf.into())
        }
    };

    let readonly = {
        let mut buf: Vec<u32> = Vec::new();
        for (idx, &readonly) in fcall_args.readonly.as_ref().iter().enumerate() {
            if readonly {
                buf.push(idx as u32);
            }
        }

        if buf.is_empty() {
            None
        } else {
            Some(buf.into())
        }
    };

    let call = instr::Call {
        context,
        detail,
        flags: fcall_args.flags,
        inouts,
        loc: ctx.loc,
        num_rets: fcall_args.num_rets,
        operands: operands.into(),
        readonly,
    };

    let num_rets = call.num_rets;
    if fcall_args.has_async_eager_target() {
        let async_bid = ctx.alloc_bid();
        let eager_bid = ctx.alloc_bid();

        let stack_size = ctx.spill_stack();
        ctx.emit(Instr::Terminator(Terminator::CallAsync(
            Box::new(call),
            [async_bid, eager_bid],
        )));

        // The eager side is tricky - we need a param to receive the produced
        // value - but then we need to spill the args and call our target.
        ctx.builder.start_block(eager_bid);
        ctx.unspill_stack(stack_size);
        ctx.alloc_and_push_param();
        let stack_size2 = ctx.spill_stack();
        let eager_bid2 = ctx.target_from_label(fcall_args.async_eager_target, stack_size2);
        ctx.emit(Instr::jmp(eager_bid2, ctx.loc));

        ctx.builder.start_block(async_bid);
        ctx.unspill_stack(stack_size);
        ctx.alloc_and_push_param();

        // And continue with the async (non-eager) block.
    } else {
        let vid = ctx.emit(Instr::call(call));

        match num_rets {
            0 => {}
            1 => ctx.push(vid),
            _ => ctx.emit_selects(vid, num_rets),
        }
    }
}

fn convert_dim<'a, 'b>(ctx: &mut Context<'a, 'b>, opcode: &Opcode<'a>) {
    if ctx.member_op.as_mut().is_none() {
        panic!("Unable to convert dim {:?} without existing base", opcode);
    };

    match *opcode {
        Opcode::Dim(mode, key) => {
            let (key, readonly, value, local) = convert_member_key(ctx, &key);
            let member_op = ctx.member_op.as_mut().unwrap();
            if let Some(value) = value {
                member_op.operands.push(value);
            }
            if let Some(local) = local {
                member_op.locals.push(local);
            }
            member_op.intermediate_ops.push(instr::IntermediateOp {
                mode,
                key,
                readonly,
                loc: ctx.loc,
            });
        }
        _ => unreachable!(),
    }
}

fn member_op_mutates_stack_base(op: &instr::MemberOp) -> bool {
    use instr::BaseOp;
    use instr::FinalOp;

    let write_op = match op.final_op {
        FinalOp::QueryM { .. } => false,
        FinalOp::SetRangeM { .. }
        | FinalOp::UnsetM { .. }
        | FinalOp::IncDecM { .. }
        | FinalOp::SetM { .. }
        | FinalOp::SetOpM { .. } => true,
    };

    let base_key_is_element_access = op.intermediate_ops.get(0).map_or_else(
        || op.final_op.key().map_or(true, |k| k.is_element_access()),
        |dim| dim.key.is_element_access(),
    );

    match op.base_op {
        BaseOp::BaseC { .. } => base_key_is_element_access && write_op,
        BaseOp::BaseGC { .. }
        | BaseOp::BaseH { .. }
        | BaseOp::BaseL { .. }
        | BaseOp::BaseSC { .. }
        | BaseOp::BaseST { .. } => false,
    }
}

fn member_key_stack_count(k: &MemberKey) -> usize {
    match k {
        MemberKey::EC | MemberKey::PC => 1,
        MemberKey::EI(_)
        | MemberKey::EL
        | MemberKey::ET(_)
        | MemberKey::PL
        | MemberKey::PT(_)
        | MemberKey::QT(_)
        | MemberKey::W => 0,
    }
}

fn convert_member_key<'a, 'b>(
    ctx: &mut Context<'a, 'b>,
    key: &hhbc::MemberKey<'a>,
) -> (
    instr::MemberKey,
    ir::ReadonlyOp,
    Option<ValueId>,
    Option<LocalId>,
) {
    match *key {
        hhbc::MemberKey::EC(idx, readonly) => {
            let vid = ctx.stack_get(idx as usize);
            (instr::MemberKey::EC, readonly, Some(vid), None)
        }
        hhbc::MemberKey::EI(i, readonly) => (instr::MemberKey::EI(i), readonly, None, None),
        hhbc::MemberKey::EL(local, readonly) => {
            let lid = convert_local(ctx, &local);
            (instr::MemberKey::EL, readonly, None, Some(lid))
        }
        hhbc::MemberKey::ET(s, readonly) => {
            let id = ctx.intern_ffi_str(s);
            (instr::MemberKey::ET(id), readonly, None, None)
        }
        hhbc::MemberKey::PC(idx, readonly) => {
            let vid = ctx.stack_get(idx as usize);
            (instr::MemberKey::PC, readonly, Some(vid), None)
        }
        hhbc::MemberKey::PL(local, readonly) => {
            let lid = convert_local(ctx, &local);
            (instr::MemberKey::PL, readonly, None, Some(lid))
        }
        hhbc::MemberKey::PT(s, readonly) => {
            let id = ir::PropId::from_hhbc(s, ctx.strings);
            (instr::MemberKey::PT(id), readonly, None, None)
        }
        hhbc::MemberKey::QT(s, readonly) => {
            let id = ir::PropId::from_hhbc(s, ctx.strings);
            (instr::MemberKey::QT(id), readonly, None, None)
        }
        hhbc::MemberKey::W => (instr::MemberKey::W, ir::ReadonlyOp::Any, None, None),
    }
}

fn convert_final<'a, 'b>(ctx: &mut Context<'a, 'b>, fin: &Opcode<'a>) {
    let mut member_op = if let Some(mop) = ctx.member_op.take() {
        mop
    } else {
        panic!("Unable to convert final {:?} without existing base", fin);
    };

    let loc = ctx.loc;
    let (member_op, pop_count, pushes_value) = match *fin {
        Opcode::IncDecM(num, inc_dec_op, key) => {
            let (key, readonly, value, local) = convert_member_key(ctx, &key);
            if let Some(value) = value {
                member_op.operands.push(value);
            }
            if let Some(local) = local {
                member_op.locals.push(local);
            }
            let mop = member_op.into_member_op(instr::FinalOp::IncDecM {
                key,
                readonly,
                inc_dec_op,
                loc,
            });
            (mop, num, true)
        }
        Opcode::QueryM(num, query_m_op, key) => {
            let (key, readonly, value, local) = convert_member_key(ctx, &key);
            if let Some(value) = value {
                member_op.operands.push(value);
            }
            if let Some(local) = local {
                member_op.locals.push(local);
            }
            let mop = member_op.into_member_op(instr::FinalOp::QueryM {
                key,
                readonly,
                query_m_op,
                loc,
            });
            (mop, num, true)
        }
        Opcode::SetM(num, key) => {
            let (key, readonly, value, local) = convert_member_key(ctx, &key);
            if let Some(value) = value {
                member_op.operands.push(value);
            }
            if let Some(local) = local {
                member_op.locals.push(local);
            }
            member_op.operands.push(ctx.pop());
            let mop = member_op.into_member_op(instr::FinalOp::SetM { key, readonly, loc });
            (mop, num, true)
        }
        Opcode::SetOpM(num, set_op_op, key) => {
            let (key, readonly, value, local) = convert_member_key(ctx, &key);
            if let Some(value) = value {
                member_op.operands.push(value);
            }
            if let Some(local) = local {
                member_op.locals.push(local);
            }
            member_op.operands.push(ctx.pop());
            let mop = member_op.into_member_op(instr::FinalOp::SetOpM {
                key,
                readonly,
                set_op_op,
                loc,
            });
            (mop, num, true)
        }
        Opcode::SetRangeM(num, sz, set_range_op) => {
            let s1 = ctx.pop();
            let s2 = ctx.pop();
            let s3 = ctx.pop();
            member_op.operands.push(s3);
            member_op.operands.push(s2);
            member_op.operands.push(s1);
            let mop = member_op.into_member_op(instr::FinalOp::SetRangeM {
                sz,
                set_range_op,
                loc,
            });
            (mop, num, false)
        }
        Opcode::UnsetM(num, key) => {
            let (key, readonly, value, local) = convert_member_key(ctx, &key);
            if let Some(value) = value {
                member_op.operands.push(value);
            }
            if let Some(local) = local {
                member_op.locals.push(local);
            }
            let mop = member_op.into_member_op(instr::FinalOp::UnsetM { key, readonly, loc });
            (mop, num, false)
        }
        _ => unreachable!(),
    };

    let expected_stack = {
        let mut count = match member_op.base_op {
            instr::BaseOp::BaseC { .. } => 1,
            instr::BaseOp::BaseGC { .. } => 1,
            instr::BaseOp::BaseH { .. } => 0,
            instr::BaseOp::BaseL { .. } => 0,
            instr::BaseOp::BaseSC { .. } => 2,
            instr::BaseOp::BaseST { .. } => 1,
        };
        for op in member_op.intermediate_ops.iter() {
            count += member_key_stack_count(&op.key);
        }
        if let Some(key) = member_op.final_op.key() {
            count += member_key_stack_count(key);
        }
        count
    };
    let mut expected_stack = ctx.pop_n(expected_stack as u32);

    let num_rets = member_op.num_values();
    let mutates_stack_base = member_op_mutates_stack_base(&member_op);
    let mut vid = ctx.emit(instr::Instr::MemberOp(member_op));

    match (num_rets, mutates_stack_base) {
        (0, false) | (1, false) => {}
        (1, true) => {
            // The base instruction mutates its stack entry but doesn't return
            // any value:
            //   BaseC
            //   UnsetM
            expected_stack[0] = vid;
        }
        (2, true) => {
            // The base instruction mutates its stack entry and also returns a
            // value:
            //   BaseC
            //   SetM
            let ret0 = ctx.emit(Instr::Special(instr::Special::Select(vid, 0)));
            let ret1 = ctx.emit(Instr::Special(instr::Special::Select(vid, 1)));
            expected_stack[0] = ret1;
            vid = ret0;
        }
        // (0, true) => mutates its stack but no returns?
        // (2, false) => multiple returns but doesn't mutate the stack?
        _ => panic!("Unexpected combination ({num_rets}, {mutates_stack_base})"),
    }

    let pop_count = pop_count as usize;

    match pop_count.cmp(&expected_stack.len()) {
        Ordering::Greater => {
            ctx.pop_n((pop_count - expected_stack.len()) as u32);
        }
        Ordering::Less => {
            expected_stack.shrink_to(expected_stack.len() - pop_count);
            ctx.push_n(expected_stack.into_iter());
        }
        Ordering::Equal => {
            // Already popped the right amount.
        }
    }

    if pushes_value {
        ctx.push(vid);
    }
}

fn convert_include<'a, 'b>(ctx: &mut Context<'a, 'b>, ie: &Opcode<'a>) {
    use instr::IncludeKind;
    let kind = match ie {
        Opcode::Eval => IncludeKind::Eval,
        Opcode::Incl => IncludeKind::Include,
        Opcode::InclOnce => IncludeKind::IncludeOnce,
        Opcode::Req => IncludeKind::Require,
        Opcode::ReqDoc => IncludeKind::RequireOnceDoc,
        Opcode::ReqOnce => IncludeKind::RequireOnce,
        _ => unreachable!(),
    };
    let vid = ctx.pop();
    let loc = ctx.loc;
    let ie = instr::IncludeEval { kind, vid, loc };
    ctx.emit_push(Instr::Hhbc(instr::Hhbc::IncludeEval(ie)));
}

fn convert_local<'a, 'b>(ctx: &mut Context<'a, 'b>, local: &hhbc::Local) -> LocalId {
    if let Some(local) = ctx.named_local_lookup.get(local.as_usize()) {
        *local
    } else {
        // Convert the hhbc::Local ID to a 0-based number.
        let id = ir::UnnamedLocalId::from_usize(local.as_usize() - ctx.named_local_lookup.len());
        LocalId::Unnamed(id)
    }
}

fn convert_local_range<'a, 'b>(
    ctx: &mut Context<'a, 'b>,
    range: &hhbc::LocalRange,
) -> Box<[LocalId]> {
    let mut locals = Vec::default();
    if range.start != hhbc::Local::INVALID && range.len != 0 {
        let size = range.len as usize;
        for idx in 0..size {
            let local = hhbc::Local::from_usize(range.start.as_usize() + idx);
            let lid = convert_local(ctx, &local);
            locals.push(lid);
        }
    }
    locals.into()
}

fn convert_iterator<'a, 'b>(ctx: &mut Context<'a, 'b>, opcode: &Opcode<'a>) {
    match *opcode {
        Opcode::IterInit(ref args, label) => {
            let hhbc::IterArgs {
                iter_id,
                ref key_id,
                ref val_id,
            } = *args;
            let key_lid = key_id.is_valid().then(|| convert_local(ctx, key_id));
            let value_lid = convert_local(ctx, val_id);
            let base_iid = ctx.pop();
            let stack_size = ctx.spill_stack();
            let next_bid = ctx.builder.alloc_bid();
            let done_bid = ctx.target_from_label(label, stack_size);

            let args =
                instr::IteratorArgs::new(iter_id, key_lid, value_lid, done_bid, next_bid, ctx.loc);
            ctx.emit(Instr::Terminator(Terminator::IterInit(args, base_iid)));
            ctx.builder.start_block(next_bid);
            ctx.unspill_stack(stack_size);
        }
        Opcode::IterNext(ref args, label) => {
            let hhbc::IterArgs {
                iter_id,
                ref key_id,
                ref val_id,
            } = *args;
            let key_lid = key_id.is_valid().then(|| convert_local(ctx, key_id));
            let value_lid = convert_local(ctx, val_id);
            let stack_size = ctx.spill_stack();
            let next_bid = ctx.builder.alloc_bid();
            let done_bid = ctx.target_from_label(label, stack_size);

            let args =
                instr::IteratorArgs::new(iter_id, key_lid, value_lid, done_bid, next_bid, ctx.loc);
            ctx.emit(Instr::Terminator(Terminator::IterNext(args)));
            ctx.builder.start_block(next_bid);
            ctx.unspill_stack(stack_size);
        }
        _ => unreachable!(),
    }
}

fn convert_control_flow<'a, 'b>(ctx: &mut Context<'a, 'b>, opcode: &Opcode<'a>) {
    let loc = ctx.loc;
    match *opcode {
        Opcode::JmpNZ(label) | Opcode::JmpZ(label) => {
            let s1 = ctx.pop();
            let stack_size = ctx.spill_stack();
            let true_bid = ctx.target_from_label(label, stack_size);
            let false_bid = ctx.alloc_bid();
            let pred = match *opcode {
                Opcode::JmpNZ(_) => ir::Predicate::NonZero,
                Opcode::JmpZ(_) => ir::Predicate::Zero,
                _ => unreachable!(),
            };
            let instr = Terminator::JmpOp {
                cond: s1,
                pred,
                targets: [true_bid, false_bid],
                loc,
            };
            ctx.emit(Instr::Terminator(instr));
            ctx.builder.start_block(false_bid);
            ctx.unspill_stack(stack_size);
        }
        Opcode::Switch(bounded, base, ref targets) => {
            let s1 = ctx.pop();
            let stack_size = ctx.spill_stack();
            let targets = targets
                .iter()
                .map(|label| ctx.target_from_label(*label, stack_size))
                .collect();
            let instr = Terminator::Switch {
                cond: s1,
                bounded,
                base,
                targets,
                loc,
            };
            ctx.emit(Instr::Terminator(instr));
        }
        Opcode::SSwitch {
            ref cases,
            ref targets,
            _0: _,
        } => {
            let s1 = ctx.pop();
            let stack_size = ctx.spill_stack();
            let cases = cases.iter().map(|case| ctx.intern_ffi_str(*case)).collect();
            let targets = targets
                .iter()
                .map(|label| ctx.target_from_label(*label, stack_size))
                .collect();
            let instr = Terminator::SSwitch {
                cond: s1,
                cases,
                targets,
                loc,
            };
            ctx.emit(Instr::Terminator(instr));
        }
        _ => unreachable!(),
    }
}

#[b2i_macros::bc_to_ir]
#[allow(clippy::todo)]
fn convert_opcode<'a, 'b>(ctx: &mut Context<'a, 'b>, opcode: &Opcode<'a>) -> bool {
    use instr::Hhbc;
    use ir::Constant;
    let loc = ctx.loc;

    enum Action<'a> {
        Emit(Instr),
        Constant(Constant<'a>),
        None,
        Push(Instr),
        Terminal(Terminator),
    }

    // (Normally Rust doesn't let you have an attribute on a statement. This
    // only works because the outer function is also wrapped with #[bc_to_ir]
    // and it looks for this.)
    //
    // The embedded macros in this match expression (simple!, todo!) use the
    // hhbc opcode table to figure out how to convert standard opcodes.
    #[bc_to_ir]
    let action = match *opcode {
        Opcode::BaseC(..)
        | Opcode::BaseGC(..)
        | Opcode::BaseGL(..)
        | Opcode::BaseSC(..)
        | Opcode::BaseL(..)
        | Opcode::BaseH => {
            convert_base(ctx, opcode);
            Action::None
        }
        Opcode::Dim(..) => {
            convert_dim(ctx, opcode);
            Action::None
        }
        Opcode::QueryM(..)
        | Opcode::SetM(..)
        | Opcode::IncDecM(..)
        | Opcode::SetOpM(..)
        | Opcode::UnsetM(..)
        | Opcode::SetRangeM(..) => {
            convert_final(ctx, opcode);
            Action::None
        }

        Opcode::FCallClsMethod { .. }
        | Opcode::FCallClsMethodD { .. }
        | Opcode::FCallClsMethodM { .. }
        | Opcode::FCallClsMethodS { .. }
        | Opcode::FCallClsMethodSD { .. }
        | Opcode::FCallCtor(..)
        | Opcode::FCallFunc(..)
        | Opcode::FCallFuncD { .. }
        | Opcode::FCallObjMethod { .. }
        | Opcode::FCallObjMethodD { .. } => {
            convert_call(ctx, opcode);
            Action::None
        }

        Opcode::Eval
        | Opcode::Incl
        | Opcode::InclOnce
        | Opcode::Req
        | Opcode::ReqDoc
        | Opcode::ReqOnce => {
            convert_include(ctx, opcode);
            Action::None
        }

        Opcode::IterInit(_, _) | Opcode::IterNext(_, _) => {
            convert_iterator(ctx, opcode);
            Action::None
        }

        Opcode::Jmp(label) => {
            let stack_size = ctx.spill_stack();
            let bid = ctx.target_from_label(label, stack_size);
            Action::Terminal(Terminator::Jmp(bid, ctx.loc))
        }
        Opcode::Enter(label) => {
            let stack_size = ctx.spill_stack();
            let bid = ctx.target_from_label(label, stack_size);
            Action::Terminal(Terminator::Enter(bid, ctx.loc))
        }

        Opcode::JmpNZ(..) | Opcode::JmpZ(..) | Opcode::Switch(..) | Opcode::SSwitch { .. } => {
            convert_control_flow(ctx, opcode);
            Action::None
        }

        // MemoGet is a terminal - but it needs to use 'push_expected_param'
        // after the push so it can't use convert_terminal().
        Opcode::MemoGet(else_label, ref locals) => {
            let stack_size = ctx.spill_stack();
            let value_bid = ctx.builder.alloc_bid();
            let else_bid = ctx.target_from_label(else_label, stack_size);
            let locals = convert_local_range(ctx, locals);
            let get = instr::MemoGet::new(value_bid, else_bid, &locals, loc);
            ctx.emit(Instr::Terminator(Terminator::MemoGet(get)));
            ctx.builder.start_block(value_bid);
            ctx.unspill_stack(stack_size);
            ctx.alloc_and_push_param();
            Action::None
        }
        Opcode::MemoGetEager([no_value_label, suspended_label], _, ref locals) => {
            let stack_size = ctx.spill_stack();
            let eager_bid = ctx.builder.alloc_bid();
            let no_value_bid = ctx.target_from_label(no_value_label, stack_size);
            let suspended_bid = ctx.alloc_bid();
            let locals = convert_local_range(ctx, locals);
            let get =
                instr::MemoGetEager::new(no_value_bid, suspended_bid, eager_bid, &locals, loc);
            ctx.emit(Instr::Terminator(Terminator::MemoGetEager(get)));

            ctx.builder.start_block(suspended_bid);
            ctx.unspill_stack(stack_size);
            ctx.alloc_and_push_param();
            let stack_size2 = ctx.spill_stack();
            let suspended_bid2 = ctx.target_from_label(suspended_label, stack_size2);
            ctx.emit(Instr::jmp(suspended_bid2, ctx.loc));

            ctx.builder.start_block(eager_bid);
            ctx.unspill_stack(stack_size);
            ctx.alloc_and_push_param();
            Action::None
        }

        Opcode::Fatal(subop1) => {
            let s1 = ctx.pop();
            // Fatal throws away the rest of the stack.
            ctx.stack_clear();
            Action::Terminal(Terminator::Fatal(s1, subop1, ctx.loc))
        }

        Opcode::ThrowAsTypeStructException => {
            let s2 = ctx.pop();
            let s1 = ctx.pop();
            ctx.stack_clear();
            Action::Terminal(Terminator::ThrowAsTypeStructException([s1, s2], ctx.loc))
        }

        Opcode::Dict(name) => {
            let tv = Arc::clone(&ctx.adata_lookup[&name]);
            debug_assert!(matches!(*tv, ir::TypedValue::Dict(_)));
            Action::Constant(Constant::Array(tv))
        }
        Opcode::Keyset(name) => {
            let tv = Arc::clone(&ctx.adata_lookup[&name]);
            debug_assert!(matches!(*tv, ir::TypedValue::Keyset(_)));
            Action::Constant(Constant::Array(tv))
        }
        Opcode::Vec(name) => {
            let tv = Arc::clone(&ctx.adata_lookup[&name]);
            debug_assert!(matches!(*tv, ir::TypedValue::Vec(_)));
            Action::Constant(Constant::Array(tv))
        }

        Opcode::AKExists => simple!(Hhbc::AKExists),
        Opcode::Add => simple!(Hhbc::Add),
        Opcode::AddElemC => simple!(Hhbc::AddElemC),
        Opcode::AddNewElemC => simple!(Hhbc::AddNewElemC),
        Opcode::ArrayIdx => simple!(Hhbc::ArrayIdx),
        Opcode::ArrayMarkLegacy => simple!(Hhbc::ArrayMarkLegacy),
        Opcode::ArrayUnmarkLegacy => simple!(Hhbc::ArrayUnmarkLegacy),
        Opcode::AssertRATL => todo!(),
        Opcode::AssertRATStk => todo!(),
        Opcode::Await => simple!(Hhbc::Await),
        Opcode::AwaitAll => simple!(Hhbc::AwaitAll),
        Opcode::BareThis => simple!(Hhbc::BareThis),
        Opcode::BitAnd => simple!(Hhbc::BitAnd),
        Opcode::BitNot => simple!(Hhbc::BitNot),
        Opcode::BitOr => simple!(Hhbc::BitOr),
        Opcode::BitXor => simple!(Hhbc::BitXor),
        Opcode::BreakTraceHint => todo!(),
        Opcode::CGetCUNop => todo!(),
        Opcode::CGetG => simple!(Hhbc::CGetG),
        Opcode::CGetL => simple!(Hhbc::CGetL),
        Opcode::CGetQuietL => simple!(Hhbc::CGetQuietL),
        Opcode::CGetS => simple!(Hhbc::CGetS),
        Opcode::CUGetL => simple!(Hhbc::CUGetL),
        Opcode::CastBool => simple!(Hhbc::CastBool),
        Opcode::CastDict => simple!(Hhbc::CastDict),
        Opcode::CastDouble => simple!(Hhbc::CastDouble),
        Opcode::CastInt => simple!(Hhbc::CastInt),
        Opcode::CastKeyset => simple!(Hhbc::CastKeyset),
        Opcode::CastString => simple!(Hhbc::CastString),
        Opcode::CastVec => simple!(Hhbc::CastVec),
        Opcode::ChainFaults => simple!(Hhbc::ChainFaults),
        Opcode::CheckProp => simple!(Hhbc::CheckProp),
        Opcode::CheckClsReifiedGenericMismatch => simple!(Hhbc::CheckClsReifiedGenericMismatch),
        Opcode::CheckClsRGSoft => simple!(Hhbc::CheckClsRGSoft),
        Opcode::CheckThis => simple!(Hhbc::CheckThis),
        Opcode::ClassGetC => simple!(Hhbc::ClassGetC),
        Opcode::ClassHasReifiedGenerics => simple!(Hhbc::ClassHasReifiedGenerics),
        Opcode::ClassName => simple!(Hhbc::ClassName),
        Opcode::Clone => simple!(Hhbc::Clone),
        Opcode::ClsCns => simple!(Hhbc::ClsCns),
        Opcode::ClsCnsD => simple!(Hhbc::ClsCnsD),
        Opcode::ClsCnsL => simple!(Hhbc::ClsCnsL),
        Opcode::Cmp => simple!(Hhbc::Cmp),
        Opcode::ColFromArray => simple!(Hhbc::ColFromArray),
        Opcode::CombineAndResolveTypeStruct => simple!(Hhbc::CombineAndResolveTypeStruct),
        Opcode::Concat => simple!(Hhbc::Concat),
        Opcode::ConcatN => simple!(Hhbc::ConcatN),
        Opcode::ContCheck => simple!(Hhbc::ContCheck),
        Opcode::ContCurrent => simple!(Hhbc::ContCurrent),
        Opcode::ContEnter => simple!(Hhbc::ContEnter),
        Opcode::ContGetReturn => simple!(Hhbc::ContGetReturn),
        Opcode::ContKey => simple!(Hhbc::ContKey),
        Opcode::ContRaise => simple!(Hhbc::ContRaise),
        Opcode::ContValid => simple!(Hhbc::ContValid),
        Opcode::CreateCont => simple!(Hhbc::CreateCont),
        Opcode::CreateSpecialImplicitContext => simple!(Hhbc::CreateSpecialImplicitContext),
        Opcode::DblAsBits => todo!(),
        Opcode::Dir => simple!(Constant::Dir),
        Opcode::Div => simple!(Hhbc::Div),
        Opcode::Double => simple!(Constant::Float),
        Opcode::EnumClassLabelName => simple!(Hhbc::EnumClassLabelName),
        Opcode::Eq => simple!(Hhbc::CmpOp, CmpOp::Eq),
        Opcode::Exit => simple!(Terminator::Exit),
        Opcode::False => simple!(Constant::Bool, false),
        Opcode::File => simple!(Constant::File),
        Opcode::FuncCred => simple!(Constant::FuncCred),
        Opcode::GetClsRGProp => simple!(Hhbc::GetClsRGProp),
        Opcode::GetMemoKeyL => simple!(Hhbc::GetMemoKeyL),
        Opcode::Gt => simple!(Hhbc::CmpOp, CmpOp::Gt),
        Opcode::Gte => simple!(Hhbc::CmpOp, CmpOp::Gte),
        Opcode::HasReifiedParent => simple!(Hhbc::HasReifiedParent),
        Opcode::Idx => simple!(Hhbc::Idx),
        Opcode::IncDecG => todo!(),
        Opcode::IncDecL => simple!(Hhbc::IncDecL),
        Opcode::IncDecS => simple!(Hhbc::IncDecS),
        Opcode::InitProp => simple!(Hhbc::InitProp),
        Opcode::InstanceOf => todo!(),
        Opcode::InstanceOfD => simple!(Hhbc::InstanceOfD),
        Opcode::Int => simple!(Constant::Int),
        Opcode::IsLateBoundCls => simple!(Hhbc::IsLateBoundCls),
        Opcode::IsTypeC => simple!(Hhbc::IsTypeC),
        Opcode::IsTypeL => simple!(Hhbc::IsTypeL),
        Opcode::IsTypeStructC => simple!(Hhbc::IsTypeStructC),
        Opcode::IsUnsetL => todo!(),
        Opcode::IssetG => simple!(Hhbc::IssetG),
        Opcode::IssetL => simple!(Hhbc::IssetL),
        Opcode::IssetS => simple!(Hhbc::IssetS),
        Opcode::IterFree => simple!(Hhbc::IterFree),
        Opcode::LIterFree => todo!(),
        Opcode::LIterInit => todo!(),
        Opcode::LIterNext => todo!(),
        Opcode::LateBoundCls => simple!(Hhbc::LateBoundCls),
        Opcode::LazyClass => simple!(Constant::LazyClass),
        Opcode::LazyClassFromClass => simple!(Hhbc::LazyClassFromClass),
        Opcode::LockObj => simple!(Hhbc::LockObj),
        Opcode::Lt => simple!(Hhbc::CmpOp, CmpOp::Lt),
        Opcode::Lte => simple!(Hhbc::CmpOp, CmpOp::Lte),
        Opcode::MemoSet => simple!(Hhbc::MemoSet),
        Opcode::MemoSetEager => simple!(Hhbc::MemoSetEager),
        Opcode::Method => simple!(Constant::Method),
        Opcode::Mod => simple!(Hhbc::Modulo),
        Opcode::Mul => simple!(Hhbc::Mul),
        Opcode::NSame => simple!(Hhbc::CmpOp, CmpOp::NSame),
        Opcode::NativeImpl => simple!(Terminator::NativeImpl),
        Opcode::Neq => simple!(Hhbc::CmpOp, CmpOp::Neq),
        Opcode::NewCol => simple!(Constant::NewCol),
        Opcode::NewDictArray => simple!(Hhbc::NewDictArray),
        Opcode::NewKeysetArray => simple!(Hhbc::NewKeysetArray),
        Opcode::NewObj => simple!(Hhbc::NewObj),
        Opcode::NewObjD => simple!(Hhbc::NewObjD),
        Opcode::NewObjS => simple!(Hhbc::NewObjS),
        Opcode::NewPair => simple!(Hhbc::NewPair),
        Opcode::NewVec => simple!(Hhbc::NewVec),
        Opcode::Nop => todo!(),
        Opcode::Not => simple!(Hhbc::Not),
        Opcode::Null => simple!(Constant::Null),
        Opcode::NullUninit => simple!(Constant::Uninit),
        Opcode::OODeclExists => simple!(Hhbc::OODeclExists),
        Opcode::ParentCls => simple!(Hhbc::ParentCls),
        Opcode::PopL => simple!(Hhbc::SetL),
        Opcode::PopU => todo!(),
        Opcode::PopU2 => todo!(),
        Opcode::Pow => simple!(Hhbc::Pow),
        Opcode::Print => simple!(Hhbc::Print),
        Opcode::PushL => simple!(Hhbc::ConsumeL),
        #[rustfmt::skip]
        Opcode::RaiseClassStringConversionNotice => simple!(Hhbc::RaiseClassStringConversionNotice),
        Opcode::RecordReifiedGeneric => simple!(Hhbc::RecordReifiedGeneric),
        Opcode::ResolveClass => simple!(Hhbc::ResolveClass),
        Opcode::ResolveClsMethod => simple!(Hhbc::ResolveClsMethod),
        Opcode::ResolveClsMethodD => simple!(Hhbc::ResolveClsMethodD),
        Opcode::ResolveClsMethodS => simple!(Hhbc::ResolveClsMethodS),
        Opcode::ResolveFunc => simple!(Hhbc::ResolveFunc),
        Opcode::ResolveMethCaller => simple!(Hhbc::ResolveMethCaller),
        Opcode::ResolveRClsMethod => simple!(Hhbc::ResolveRClsMethod),
        Opcode::ResolveRClsMethodD => simple!(Hhbc::ResolveRClsMethodD),
        Opcode::ResolveRClsMethodS => simple!(Hhbc::ResolveRClsMethodS),
        Opcode::ResolveRFunc => simple!(Hhbc::ResolveRFunc),
        Opcode::RetC => simple!(Terminator::Ret),
        Opcode::RetCSuspended => simple!(Terminator::RetCSuspended),
        Opcode::RetM => simple!(Terminator::RetM),
        Opcode::Same => simple!(Hhbc::CmpOp, CmpOp::Same),
        Opcode::Select => todo!(),
        Opcode::SelfCls => simple!(Hhbc::SelfCls),
        Opcode::SetG => simple!(Hhbc::SetG),
        Opcode::SetL => simple!(Hhbc::SetL),
        Opcode::SetImplicitContextByValue => simple!(Hhbc::SetImplicitContextByValue),
        Opcode::SetOpG => simple!(Hhbc::SetOpG),
        Opcode::SetOpL => simple!(Hhbc::SetOpL),
        Opcode::SetOpS => simple!(Hhbc::SetOpS),
        Opcode::SetS => simple!(Hhbc::SetS),
        Opcode::Shl => simple!(Hhbc::Shl),
        Opcode::Shr => simple!(Hhbc::Shr),
        Opcode::Silence => simple!(Hhbc::Silence),
        Opcode::Sub => simple!(Hhbc::Sub),
        Opcode::This => simple!(Hhbc::This),
        Opcode::Throw => simple!(Terminator::Throw),
        Opcode::ThrowNonExhaustiveSwitch => simple!(Hhbc::ThrowNonExhaustiveSwitch),
        Opcode::True => simple!(Constant::Bool, true),
        Opcode::UGetCUNop => todo!(),
        Opcode::UnsetG => simple!(Hhbc::UnsetG),
        Opcode::UnsetL => simple!(Hhbc::UnsetL),
        Opcode::VerifyImplicitContextState => simple!(Hhbc::VerifyImplicitContextState),
        Opcode::VerifyOutType => simple!(Hhbc::VerifyOutType),
        Opcode::VerifyParamType => simple!(Hhbc::VerifyParamType),
        Opcode::VerifyParamTypeTS => simple!(Hhbc::VerifyParamTypeTS),
        Opcode::VerifyRetNonNullC => todo!(),
        Opcode::VerifyRetTypeC => simple!(Hhbc::VerifyRetTypeC),
        Opcode::VerifyRetTypeTS => simple!(Hhbc::VerifyRetTypeTS),
        Opcode::WHResult => simple!(Hhbc::WHResult),
        Opcode::Yield => simple!(Hhbc::Yield),
        Opcode::YieldK => simple!(Hhbc::YieldK),

        Opcode::ClassGetTS => {
            let s1 = ctx.pop();
            let vid = ctx.emit(Instr::Hhbc(Hhbc::ClassGetTS(s1, ctx.loc)));
            ctx.emit_selects(vid, 2);
            Action::None
        }

        Opcode::CGetL2(ref local) => {
            let lid = convert_local(ctx, local);
            let s1 = ctx.pop();
            ctx.emit_push(Instr::Hhbc(Hhbc::CGetL(lid, loc)));
            ctx.push(s1);
            Action::None
        }
        Opcode::CnsE(id) => Action::Constant(Constant::Named(id)),
        Opcode::CreateCl(num_args, class) => {
            let operands = collect_args(ctx, num_args);
            let clsid = ir::ClassId::from_hhbc(class, ctx.strings);
            Action::Push(Instr::Hhbc(Hhbc::CreateCl {
                operands: operands.into(),
                clsid,
                loc,
            }))
        }
        Opcode::Dup => {
            let s1 = ctx.pop();
            ctx.push(s1);
            ctx.push(s1);
            Action::None
        }
        Opcode::NewStructDict(keys) => {
            let keys: Box<[UnitBytesId]> = keys
                .iter()
                .map(|key| ctx.strings.intern_bytes(key.as_ref()))
                .collect();
            let values = collect_args(ctx, keys.len() as u32);
            Action::Push(Instr::Hhbc(Hhbc::NewStructDict(keys, values.into(), loc)))
        }
        Opcode::PopC => {
            ctx.pop();
            Action::None
        }
        Opcode::String(value) => {
            let s1 = Constant::String(ctx.strings.intern_bytes(value.as_ref()));
            Action::Constant(s1)
        }
        Opcode::EnumClassLabel(value) => {
            let s1 = Constant::EnumClassLabel(ctx.strings.intern_bytes(value.as_ref()));
            Action::Constant(s1)
        }
    };

    match action {
        Action::Emit(opcode) => {
            ctx.emit(opcode);
        }
        Action::Constant(constant) => {
            ctx.emit_push_constant(constant);
        }
        Action::None => {}
        Action::Push(opcode) => {
            ctx.emit_push(opcode);
        }
        Action::Terminal(terminal) => {
            ctx.emit(Instr::Terminator(terminal));
            return false;
        }
    }

    true
}

fn add_catch_work<'a, 'b>(ctx: &mut Context<'a, 'b>, mut tcid: TryCatchId) {
    loop {
        match tcid {
            TryCatchId::None => {
                // No catch target.
                return;
            }
            TryCatchId::Try(exid) => {
                // Catch block is the catch of this exid.
                ctx.add_work_bid(ctx.builder.func.ex_frames[&exid].catch_bid);
                return;
            }
            TryCatchId::Catch(exid) => {
                // Catch block is the catch of the parent. If our parent is a
                // Try(_) then we want its catch. If our parent is a Catch(_)
                // then we want its parent's stuff.
                tcid = ctx.builder.func.ex_frames[&exid].parent;
            }
        }
    }
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::str::FromStr;

use anyhow::Error;
use ascii::AsciiString;
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
use ir::StringInterner;
use ir::TryCatchId;
use ir::ValueId;
use itertools::Itertools;
use log::trace;

use crate::class;
use crate::hack;
use crate::mangle::Mangle as _;
use crate::mangle::MangleWithClass as _;
use crate::state::FuncDeclKind;
use crate::state::FuncDecls;
use crate::state::UnitState;
use crate::textual;
use crate::textual::Sid;
use crate::types::convert_ty;
use crate::util;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// Functions are defined as taking a param bundle.
///
/// f(params: HackParams): mixed;
pub(crate) fn write_function(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    function: ir::Function<'_>,
) -> Result {
    trace!("Convert Function {}", function.name.as_bstr());

    write_func(
        w,
        state,
        &function.name.mangle(&state.strings),
        tx_ty!(*void),
        function.func,
    )
}

pub(crate) fn write_func(
    w: &mut dyn std::io::Write,
    unit_state: &mut UnitState,
    name: &str,
    this_ty: textual::Ty,
    func: ir::Func<'_>,
) -> Result {
    let func = func.clone();
    let mut func = crate::lower::lower(func, &mut unit_state.strings);
    ir::verify::verify_func(&func, &Default::default(), &unit_state.strings)?;

    let params = std::mem::take(&mut func.params);
    let mut params = params
        .into_iter()
        .map(|p| {
            let name_bytes = unit_state.strings.lookup_bytes(p.name);
            let name_string = util::escaped_string(name_bytes);
            (name_string, convert_ty(p.ty.enforced))
        })
        .collect_vec();

    // Prepend the 'this' parameter.
    let this_name = AsciiString::from_str("this").unwrap();
    params.insert(0, (this_name, this_ty));

    let params = params
        .iter()
        .map(|(name, ty)| (name.as_str(), ty.clone()))
        .collect_vec();

    let ret_ty = convert_ty(std::mem::take(&mut func.return_type.enforced));
    let span = func.loc(func.loc_id).clone();
    let func_declares =
        textual::write_function(w, &unit_state.strings, name, &span, &params, ret_ty, |w| {
            let func = rewrite_prelude(func);
            let mut func = rewrite_jmp_ops(func);
            ir::passes::clean::run(&mut func);

            let mut state = FuncState::new(&unit_state.strings, &func);

            for bid in func.block_ids() {
                write_block(w, &mut state, bid)?;
            }

            Ok(state.func_declares)
        })?;

    unit_state
        .func_declares
        .declare(name, FuncDeclKind::Internal);
    unit_state.func_declares.merge(func_declares);

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
    // The entry BID is always included for us.
    if bid != Func::ENTRY_BID {
        w.write_label(bid, &params)?;
    }

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
            let pred = hack::expr_builtin(hack::Builtin::IsTrue, [state.lookup_vid(vid)]);
            w.prune_not(pred)?;
        }
        Instr::Special(Special::Textual(Textual::AssertTrue(vid, _))) => {
            // I think "prune" means "stop if this expression IS NOT true"...
            let pred = hack::expr_builtin(hack::Builtin::IsTrue, [state.lookup_vid(vid)]);
            w.prune(pred)?;
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
        Instr::Terminator(Terminator::Unreachable) => {
            w.unreachable()?;
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
                state.lookup_vid(vid)
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
    let sid = w.load(tx_ty!(mixed), textual::Expr::deref(lid))?;
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
        tx_ty!(mixed),
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

    assert!(state.strings.lookup_bytes(context).is_empty());
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

    let args = detail.args(operands);

    let output = match *detail {
        CallDetail::FCallClsMethod { .. } => todo!(),
        CallDetail::FCallClsMethodD { clsid, method } => {
            // C::foo()
            let target = method.mangle(clsid, state.strings);
            state
                .func_declares
                .declare(target.to_string(), FuncDeclKind::External);
            let this = class::load_static_class(w, clsid, state.strings)?;
            w.call_static(
                &target,
                this.into(),
                args.iter().copied().map(|vid| state.lookup_vid(vid)),
            )?
        }
        CallDetail::FCallClsMethodM { .. } => todo!(),
        CallDetail::FCallClsMethodS { .. } => todo!(),
        CallDetail::FCallClsMethodSD { .. } => todo!(),
        CallDetail::FCallCtor => todo!(),
        CallDetail::FCallFunc => todo!(),
        CallDetail::FCallFuncD { func } => {
            let target = func.mangle(state.strings);
            state
                .func_declares
                .declare(target.to_string(), FuncDeclKind::External);
            // A top-level function is called like a class static in a special
            // top-level class. Its 'this' pointer is null.
            w.call_static(
                &target,
                textual::Expr::null(),
                args.iter().copied().map(|vid| state.lookup_vid(vid)),
            )?
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
        IncDecOp::PreInc => hack::Builtin::Hhbc(hack::Hhbc::Add),
        IncDecOp::PostInc => hack::Builtin::Hhbc(hack::Hhbc::Add),
        IncDecOp::PreDec => hack::Builtin::Hhbc(hack::Hhbc::Sub),
        IncDecOp::PostDec => hack::Builtin::Hhbc(hack::Hhbc::Sub),
        _ => unreachable!(),
    };

    let pre = w.load(tx_ty!(mixed), textual::Expr::deref(lid))?;
    let post = hack::call_builtin(w, builtin, (pre, textual::Expr::hack_int(1)))?;
    w.store(textual::Expr::deref(lid), post, tx_ty!(mixed))?;

    let sid = match op {
        IncDecOp::PreInc | IncDecOp::PreDec => pre,
        IncDecOp::PostInc | IncDecOp::PostDec => post,
        _ => unreachable!(),
    };
    state.set_iid(iid, sid);

    Ok(())
}

pub(crate) struct FuncState<'a> {
    func_declares: FuncDecls,
    func: &'a ir::Func<'a>,
    iid_mapping: ir::InstrIdMap<Sid>,
    pub(crate) strings: &'a StringInterner,
}

impl<'a> FuncState<'a> {
    fn new(strings: &'a StringInterner, func: &'a ir::Func<'a>) -> Self {
        Self {
            func_declares: Default::default(),
            func,
            iid_mapping: Default::default(),
            strings,
        }
    }

    pub fn alloc_sid_for_iid(&mut self, w: &mut textual::FuncWriter<'_>, iid: InstrId) -> Sid {
        let sid = w.alloc_sid();
        self.set_iid(iid, sid);
        sid
    }

    /// Look up a ValueId in the FuncState and return an Expr representing
    /// it. For InstrIds and complex ConstIds return an Expr containing the
    /// (already emitted) Sid. For simple ConstIds use an Expr representing the
    /// value directly.
    pub fn lookup_vid(&self, vid: ValueId) -> textual::Expr {
        use textual::Expr;
        match vid.full() {
            ir::FullInstrId::Instr(iid) => Expr::Sid(self.lookup_iid(iid)),
            ir::FullInstrId::Constant(c) => {
                use hack::Builtin;
                let c = self.func.constant(c);
                match c {
                    Constant::Bool(v) => hack::expr_builtin(Builtin::Bool, [Expr::bool_(*v)]),
                    Constant::Int(i) => hack::expr_builtin(Builtin::Int, [Expr::int(*i)]),
                    Constant::Null => hack::expr_builtin(Builtin::Null, ()),
                    Constant::String(s) => {
                        let s = util::escaped_string(s);
                        hack::expr_builtin(Builtin::String, [Expr::string(s)])
                    }
                    Constant::Array(..) => todo!(),
                    Constant::Dir => todo!(),
                    Constant::Double(..) => todo!(),
                    Constant::File => todo!(),
                    Constant::FuncCred => todo!(),
                    Constant::Method => todo!(),
                    Constant::Named(..) => todo!(),
                    Constant::NewCol(..) => todo!(),
                    Constant::Uninit => todo!(),
                }
            }
            ir::FullInstrId::None => unreachable!(),
        }
    }

    pub fn lookup_iid(&self, iid: InstrId) -> Sid {
        *self.iid_mapping.get(&iid).unwrap()
    }

    pub(crate) fn set_iid(&mut self, iid: InstrId, sid: Sid) {
        let old = self.iid_mapping.insert(iid, sid);
        assert!(old.is_none());
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
/// - Convert complex constants into builtins.
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
    for (lid, constant) in builder.func.constants.iter().enumerate() {
        let lid = ConstantId::from_usize(lid);
        trace!("    Const {lid}: {constant:?}");
        let src = ValueId::from_constant(lid);
        let vid = match constant {
            Constant::Bool(..)
            | Constant::Double(..)
            | Constant::Int(..)
            | Constant::Null
            | Constant::String(..)
            | Constant::Uninit => None,

            Constant::Array(..) => todo!(),
            Constant::Dir => todo!(),
            Constant::File => todo!(),
            Constant::FuncCred => todo!(),
            Constant::Method => todo!(),
            Constant::Named(..) => todo!(),
            Constant::NewCol(..) => todo!(),
        };
        if let Some(vid) = vid {
            remap.insert(src, vid);
        }
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

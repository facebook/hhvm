// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashSet;
use std::str::FromStr;
use std::sync::Arc;

use anyhow::anyhow;
use anyhow::Error;
use ascii::AsciiString;
use ffi::Str;
use ir::instr::HasLoc;
use ir::instr::HasLocals;
use ir::instr::Hhbc;
use ir::instr::Predicate;
use ir::instr::Special;
use ir::instr::Terminator;
use ir::instr::Textual;
use ir::Block;
use ir::BlockId;
use ir::ClassName;
use ir::Constant;
use ir::ConstantId;
use ir::Func;
use ir::IncDecOp;
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
use crate::lower::func_builder::FuncBuilderEx as _;
use crate::mangle::Mangle as _;
use crate::mangle::MangleWithClass as _;
use crate::state;
use crate::state::FuncDeclKind;
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
        None,
    )
}

pub(crate) fn write_func(
    w: &mut dyn std::io::Write,
    unit_state: &mut UnitState,
    name: &str,
    this_ty: textual::Ty,
    func: ir::Func<'_>,
    method_info: Option<&MethodInfo<'_>>,
) -> Result {
    let mut func = crate::lower::lower_func(func, method_info, Arc::clone(&unit_state.strings));
    ir::verify::verify_func(&func, &Default::default(), &unit_state.strings)?;

    let params = std::mem::take(&mut func.params);
    let param_lids = params
        .iter()
        .map(|p| LocalId::Named(p.name))
        .collect::<HashSet<_>>();
    let mut params = params
        .into_iter()
        .map(|p| {
            let name_bytes = unit_state.strings.lookup_bytes(p.name);
            let name_string = util::escaped_string(&name_bytes);
            (name_string, convert_ty(p.ty.enforced, &unit_state.strings))
        })
        .collect_vec();

    // Prepend the 'this' parameter.
    let this_name = AsciiString::from_str("this").unwrap();
    params.insert(0, (this_name, this_ty));

    let params = params
        .iter()
        .map(|(name, ty)| (name.as_str(), ty.clone()))
        .collect_vec();

    let ret_ty = convert_ty(
        std::mem::take(&mut func.return_type.enforced),
        &unit_state.strings,
    );

    let lids = func
        .body_instrs()
        .flat_map(HasLocals::locals)
        .cloned()
        .collect::<HashSet<_>>();
    let locals = lids
        .into_iter()
        .filter(|lid| !param_lids.contains(lid))
        .sorted_by(|x, y| cmp_lid(&unit_state.strings, x, y))
        .map(|lid| {
            // TODO(arr): figure out how to provide more precise types
            let ty = tx_ty!(*void);
            (lid, ty)
        })
        .collect::<Vec<_>>();

    let span = func.loc(func.loc_id).clone();
    let decls = textual::write_function(
        w,
        &unit_state.strings,
        name,
        &span,
        &params,
        ret_ty,
        &locals,
        |w| {
            let func = rewrite_prelude(func, Arc::clone(&unit_state.strings));
            trace!(
                "After Rewrite Prelude: {}",
                ir::print::DisplayFunc(&func, true, &unit_state.strings)
            );

            let mut func = rewrite_jmp_ops(func);
            ir::passes::clean::run(&mut func);

            let mut state = FuncState::new(&unit_state.strings, &func, method_info);

            for bid in func.block_ids() {
                write_block(w, &mut state, bid)?;
            }

            Ok(state.decls)
        },
    )?;

    unit_state.decls.declare_func(name, FuncDeclKind::Internal);
    unit_state.decls.merge(decls);

    Ok(())
}

fn write_block(w: &mut textual::FuncWriter<'_>, state: &mut FuncState<'_>, bid: BlockId) -> Result {
    trace!("  Block {bid}");
    let block = state.func.block(bid);

    if block.tcid != TryCatchId::None {
        textual_todo! { w.comment("TODO: Try-Catch Block")?; }
    }

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
        Instr::Hhbc(Hhbc::SetL(vid, lid, _)) => {
            write_set_var(w, state, lid, vid)?;
            // SetL emits the input as the output.
            state.copy_iid(iid, vid);
        }
        Instr::Hhbc(Hhbc::This(_)) => write_load_this(w, state, iid)?,
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
            ref values,
            loc: _,
        })) => write_builtin(w, state, iid, target, values)?,
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

        Instr::Special(Special::Copy(..)) => todo!(),
        Instr::Special(Special::IrToBc(..)) => todo!(),
        Instr::Special(Special::Param) => todo!(),
        Instr::Special(Special::Select(..)) => todo!(),
        Instr::Special(Special::Tmp(..)) => todo!(),
        Instr::Special(Special::Tombstone) => todo!(),

        Instr::Terminator(Terminator::CallAsync(..))
        | Instr::Terminator(Terminator::Exit(..))
        | Instr::Terminator(Terminator::Fatal(..))
        | Instr::Terminator(Terminator::IterInit(..))
        | Instr::Terminator(Terminator::IterNext(..))
        | Instr::Terminator(Terminator::MemoGet(..))
        | Instr::Terminator(Terminator::MemoGetEager(..))
        | Instr::Terminator(Terminator::NativeImpl(..))
        | Instr::Terminator(Terminator::RetCSuspended(..))
        | Instr::Terminator(Terminator::RetM(..))
        | Instr::Terminator(Terminator::SSwitch { .. })
        | Instr::Terminator(Terminator::Switch { .. })
        | Instr::Terminator(Terminator::ThrowAsTypeStructException { .. }) => {
            w.write_todo(&format!("{:?}", instr))?;
        }

        Instr::Terminator(Terminator::Throw(vid, _)) => {
            textual_todo! {
                let expr = state.lookup_vid(vid);
                w.call("TODO_throw", [expr])?;
                w.unreachable()?;
            }
        }

        Instr::Hhbc(ref hhbc) => {
            // This should only handle instructions that can't be rewritten into
            // a simpler form (like control flow and generic calls). Everything
            // else should be handled in lower().
            trace!("TODO: {hhbc:?}");
            textual_todo! {
                use ir::instr::HasOperands;
                let name = format!("TODO_hhbc_{}", hhbc);
                state
                    .decls
                    .declare_func(&name, FuncDeclKind::External);
                let output = w.call(
                    &name,
                    instr
                        .operands()
                        .iter()
                        .map(|vid| state.lookup_vid(*vid))
                        .collect_vec(),
                )?;
                state.set_iid(iid, output);
            }
        }
    }

    Ok(())
}

fn write_builtin(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
    target: &str,
    values: &[ValueId],
) -> Result {
    let params = values
        .iter()
        .map(|vid| state.lookup_vid(*vid))
        .collect_vec();
    let output = w.call(target, params)?;
    state.set_iid(iid, output);
    Ok(())
}

fn write_load_this(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
) -> Result {
    let class = state.method_info.unwrap().class;
    let sid = w.load(class::non_static_ty(class.name, state.strings), "this")?;
    state.set_iid(iid, sid);
    Ok(())
}

fn write_load_var(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
    lid: LocalId,
) -> Result {
    let sid = w.load(tx_ty!(*HackMixed), textual::Expr::deref(lid))?;
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
        tx_ty!(*HackMixed),
    )
}

fn write_call(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
    call: &ir::Call,
) -> Result {
    use ir::instr::CallDetail;
    use ir::FCallArgsFlags;

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

    assert!(inouts.as_ref().map_or(true, |inouts| inouts.is_empty()));
    assert!(readonly.as_ref().map_or(true, |ro| ro.is_empty()));
    assert!(num_rets < 2);

    let context = state.strings.lookup_bytes_or_none(context);
    if let Some(context) = context {
        if !context.is_empty() {
            textual_todo! {
                w.comment("TODO: write_call(Context: {context:?})")?;
            }
        }
    }

    if flags & FCallArgsFlags::HasUnpack != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::HasUnpack")?;
        }
    }
    if flags & FCallArgsFlags::HasGenerics != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::HasGenerics")?;
        }
    }
    if flags & FCallArgsFlags::LockWhileUnwinding != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::LockWhileUnwinding")?;
        }
    }
    if flags & FCallArgsFlags::SkipRepack != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::SkipRepack")?;
        }
    }
    if flags & FCallArgsFlags::SkipCoeffectsCheck != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::SkipCoeffectsCheck")?;
        }
    }
    if flags & FCallArgsFlags::EnforceMutableReturn != 0 {
        // todo!();
    }
    if flags & FCallArgsFlags::EnforceReadonlyThis != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::EnforceReadonlyThis")?;
        }
    }
    if flags & FCallArgsFlags::ExplicitContext != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::ExplicitContext")?;
        }
    }
    if flags & FCallArgsFlags::HasInOut != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::HasInOut")?;
        }
    }
    if flags & FCallArgsFlags::EnforceInOut != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::EnforceInOut")?;
        }
    }
    if flags & FCallArgsFlags::EnforceReadonly != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::EnforceReadonly")?;
        }
    }
    if flags & FCallArgsFlags::HasAsyncEagerOffset != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::HasAsyncEagerOffset")?;
        }
    }
    if flags & FCallArgsFlags::NumArgsStart != 0 {
        textual_todo! {
            w.comment("TODO: FCallArgsFlags::NumArgsStart")?;
        }
    }

    let args = detail.args(operands);

    let output = match *detail {
        CallDetail::FCallClsMethod { .. } => todo!(),
        CallDetail::FCallClsMethodD { clsid, method } => {
            // C::foo()
            let target = method.mangle(clsid, state.strings);
            state
                .decls
                .declare_func(target.to_string(), FuncDeclKind::External);
            let this = class::load_static_class(w, clsid, state.strings, &mut state.decls)?;
            w.call_static(
                &target,
                this.into(),
                args.iter().copied().map(|vid| state.lookup_vid(vid)),
            )?
        }
        CallDetail::FCallClsMethodM { .. } => todo!(),
        CallDetail::FCallClsMethodS { .. } => todo!(),
        CallDetail::FCallClsMethodSD { .. } => {
            textual_todo! { w.call("TODO_FCallClsMethodSD", ())? }
        }
        CallDetail::FCallCtor => {
            textual_todo! {
                // new $x
                let ty = ClassName::new(Str::new(b"HackMixed"));
                let target =
                    ir::MethodName::new(ffi::Slice::new(b"TODO_ctor")).mangle(&ty, state.strings);
                state
                    .decls
                    .declare_func(target.to_string(), FuncDeclKind::External);
                w.call_virtual(
                    &target,
                    state.lookup_vid(detail.obj(operands)),
                    args.iter().copied().map(|vid| state.lookup_vid(vid)),
                )?
            }
        }
        CallDetail::FCallFunc => todo!(),
        CallDetail::FCallFuncD { func } => {
            // foo()
            let target = func.mangle(state.strings);
            state
                .decls
                .declare_func(target.to_string(), FuncDeclKind::External);
            // A top-level function is called like a class static in a special
            // top-level class. Its 'this' pointer is null.
            w.call_static(
                &target,
                textual::Expr::null(),
                args.iter().copied().map(|vid| state.lookup_vid(vid)),
            )?
        }
        CallDetail::FCallObjMethod { .. } => todo!(),
        CallDetail::FCallObjMethodD { flavor, method } => {
            // $x->y()
            if flavor == ir::ObjMethodOp::NullSafe {
                // Handle this in lowering.
                textual_todo! {
                    w.comment("TODO: NullSafe")?;
                }
            }

            // TODO: need to try to figure out the type.
            let ty = ClassName::new(Str::new(b"HackMixed"));
            let target = method.mangle(&ty, state.strings);
            state
                .decls
                .declare_func(target.to_string(), FuncDeclKind::External);
            w.call_virtual(
                &target,
                state.lookup_vid(detail.obj(operands)),
                args.iter().copied().map(|vid| state.lookup_vid(vid)),
            )?
        }
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

    let pre = w.load(tx_ty!(*HackMixed), textual::Expr::deref(lid))?;
    let one = hack::call_builtin(w, hack::Builtin::Int, [1])?;
    let post = hack::call_builtin(w, builtin, (pre, one))?;
    w.store(textual::Expr::deref(lid), post, tx_ty!(*HackMixed))?;

    let sid = match op {
        IncDecOp::PreInc | IncDecOp::PreDec => pre,
        IncDecOp::PostInc | IncDecOp::PostDec => post,
        _ => unreachable!(),
    };
    state.set_iid(iid, sid);

    Ok(())
}

pub(crate) struct FuncState<'a> {
    decls: state::Decls,
    func: &'a ir::Func<'a>,
    iid_mapping: ir::InstrIdMap<textual::Expr>,
    method_info: Option<&'a MethodInfo<'a>>,
    pub(crate) strings: &'a StringInterner,
}

impl<'a> FuncState<'a> {
    fn new(
        strings: &'a StringInterner,
        func: &'a ir::Func<'a>,
        method_info: Option<&'a MethodInfo<'a>>,
    ) -> Self {
        Self {
            decls: Default::default(),
            func,
            iid_mapping: Default::default(),
            method_info,
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
        match vid.full() {
            ir::FullInstrId::Instr(iid) => self.lookup_iid(iid),
            ir::FullInstrId::Constant(c) => {
                use hack::Builtin;
                let c = self.func.constant(c);
                match c {
                    Constant::Bool(false) => hack::expr_builtin(Builtin::Bool, [false]),
                    Constant::Bool(true) => hack::expr_builtin(Builtin::Bool, [true]),
                    Constant::Int(i) => hack::expr_builtin(Builtin::Int, [*i]),
                    Constant::Null => hack::expr_builtin(Builtin::Null, ()),
                    Constant::String(s) => {
                        let s = self.strings.lookup_bstr(*s);
                        let s = util::escaped_string(&s);
                        hack::expr_builtin(Builtin::String, [s])
                    }
                    Constant::Array(..) => todo!(),
                    Constant::Dir => todo!(),
                    Constant::Double(f) => hack::expr_builtin(Builtin::Float, [f.to_f64()]),
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

    pub fn lookup_iid(&self, iid: InstrId) -> textual::Expr {
        self.iid_mapping
            .get(&iid)
            .cloned()
            .ok_or_else(|| anyhow!("looking for {iid:?}"))
            .unwrap()
    }

    pub(crate) fn set_iid(&mut self, iid: InstrId, expr: impl Into<textual::Expr>) {
        let expr = expr.into();
        let old = self.iid_mapping.insert(iid, expr);
        assert!(old.is_none());
    }

    pub(crate) fn copy_iid(&mut self, iid: InstrId, input: ValueId) {
        let expr = self.lookup_vid(input);
        self.set_iid(iid, expr);
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
fn rewrite_prelude<'a>(mut func: ir::Func<'a>, strings: Arc<StringInterner>) -> ir::Func<'a> {
    let mut remap = ir::ValueIdMap::default();
    ir::FuncBuilder::borrow_func(&mut func, strings, |builder| {
        // Swap out the initial block so we can inject our entry code.
        let entry_bid = builder.func.alloc_bid(Block::default());
        builder.func.blocks.swap(Func::ENTRY_BID, entry_bid);
        builder
            .func
            .remap_bids(&[(Func::ENTRY_BID, entry_bid)].into_iter().collect());

        builder.start_block(Func::ENTRY_BID);
        write_constants(&mut remap, builder);
        builder.emit(Instr::jmp(entry_bid, LocId::NONE));
    });
    func.remap_vids(&remap);
    func
}

fn write_constants(remap: &mut ir::ValueIdMap<ValueId>, builder: &mut ir::FuncBuilder<'_>) {
    // Steal the contents of the "complex" constants first. We need to do this
    // because we may create more constants during lowering (but don't create
    // more complex ones!).
    let mut constants = Vec::default();
    for (lid, constant) in builder.func.constants.iter_mut().enumerate() {
        let lid = ConstantId::from_usize(lid);
        match constant {
            // Simple constants that are just emitted inline.
            Constant::Bool(..)
            | Constant::Dir
            | Constant::Double(..)
            | Constant::File
            | Constant::FuncCred
            | Constant::Int(..)
            | Constant::Method
            | Constant::Named(..)
            | Constant::NewCol(..)
            | Constant::Null
            | Constant::String(..)
            | Constant::Uninit => continue,

            // Complex constants that are emitted early.
            Constant::Array(..) => {
                let constant = std::mem::replace(constant, Constant::Uninit);
                constants.push((lid, constant));
            }
        }
    }

    for (lid, constant) in constants.into_iter() {
        trace!("    Const {lid}: {constant:?}");
        let src = ValueId::from_constant(lid);
        let loc = LocId::NONE;
        let vid = match constant {
            Constant::Bool(..)
            | Constant::Dir
            | Constant::Double(..)
            | Constant::File
            | Constant::FuncCred
            | Constant::Int(..)
            | Constant::Method
            | Constant::Named(..)
            | Constant::NewCol(..)
            | Constant::Null
            | Constant::String(..)
            | Constant::Uninit => unreachable!(),

            Constant::Array(..) => builder.emit_todo_instr(&format!("{constant:?}"), loc),
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

pub(crate) struct MethodInfo<'a> {
    pub(crate) class: &'a ir::Class<'a>,
    pub(crate) is_static: bool,
}

/// Compare locals such that named ones go first followed by unnamed ones.
/// Ordering for named locals is stable and is based on their source names.
/// Unnamed locals have only their id which may differ accross runs. In which
/// case the IR would be non-deterministic and hence unstable ordering would be
/// the least of our concerns.
fn cmp_lid(strings: &StringInterner, x: &LocalId, y: &LocalId) -> std::cmp::Ordering {
    match (x, y) {
        (LocalId::Named(x_bid), LocalId::Named(y_bid)) => {
            let x_name = strings.lookup_bytes(*x_bid);
            let y_name = strings.lookup_bytes(*y_bid);
            x_name.cmp(&y_name)
        }
        (LocalId::Named(_), LocalId::Unnamed(_)) => std::cmp::Ordering::Less,
        (LocalId::Unnamed(_), LocalId::Named(_)) => std::cmp::Ordering::Greater,
        (LocalId::Unnamed(x_id), LocalId::Unnamed(y_id)) => x_id.cmp(y_id),
    }
}

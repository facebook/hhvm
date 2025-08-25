// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::VecDeque;
use std::fmt::Display;

use hash::HashMap;
use hhbc::AdataState;
use hhbc::BytesId;
use hhbc::Dummy;
use hhbc::Instruct;
use hhbc::Opcode;
use hhbc::Pseudo;
use instruction_sequence::InstrSeq;
use ir::BlockId;
use ir::BlockIdMap;
use ir::FCallArgsFlags;
use ir::Immediate;
use ir::LocalId;
use ir::StringId;
use ir::instr;
use ir::instr::HasLoc;
use ir::instr::HasLocals;
use ir::instr::IrToBc;
use ir::print::FmtRawVid;
use ir::print::FmtSep;
use itertools::Itertools;
use log::trace;

use crate::ex_frame::BlockIdOrExFrame;
use crate::ex_frame::ExFrame;

pub(crate) fn emit_func(
    func: &ir::Func,
    labeler: &mut Labeler,
    adata_cache: &mut AdataState,
) -> (InstrSeq, Vec<StringId>) {
    let mut intern_adata = |imm: &Immediate| {
        // Identical array literals will be shared between funcs in the same unit.
        adata_cache
            .intern_value(imm.clone().try_into().unwrap())
            .clone()
    };
    let imm_to_opcode =
        ImmToOpcode::new_from_vec(Vec::from_iter(func.repr.imms.iter().map(|imm| match imm {
            Immediate::Bool(false) => Opcode::False,
            Immediate::Bool(true) => Opcode::True,
            Immediate::Dir => Opcode::Dir,
            Immediate::EnumClassLabel(v) => Opcode::EnumClassLabel(*v),
            Immediate::Float(v) => Opcode::Double(*v),
            Immediate::File => Opcode::File,
            Immediate::FuncCred => Opcode::FuncCred,
            Immediate::Int(v) => Opcode::Int(*v),
            Immediate::LazyClass(cid) => Opcode::LazyClass(*cid),
            Immediate::Method => Opcode::Method,
            Immediate::Named(name) => Opcode::CnsE(*name),
            Immediate::NewCol(k) => Opcode::NewCol(*k),
            Immediate::Null => Opcode::Null,
            Immediate::String(v) => Opcode::String(*v),
            Immediate::Uninit => Opcode::NullUninit,
            Immediate::Vec(_) => Opcode::Vec(intern_adata(imm)),
            Immediate::Dict(_) => Opcode::Dict(intern_adata(imm)),
            Immediate::Keyset(_) => Opcode::Keyset(intern_adata(imm)),
        })));

    let mut ctx = InstrEmitter::new(func, labeler, &imm_to_opcode);

    // Collect the Blocks, grouping them into TryCatch sections.
    let root = crate::ex_frame::collect_tc_sections(func);

    ctx.convert_frame_vec(root);
    // Collect the decl_vars - these are all the named locals after the params.
    let decl_vars: Vec<StringId> = ctx
        .locals
        .iter()
        .sorted_by_key(|(_, v)| *v)
        .filter_map(|(k, _)| match *k {
            LocalId::Named(name) => Some(name),
            LocalId::Unnamed(_) => None,
        })
        .skip(func.repr.params.len())
        .collect();

    (InstrSeq::List(ctx.instrs), decl_vars)
}

/// Use to look up a ImmId and get the opcode Opcode that will construct it
type ImmToOpcode = ir::IdVec<ir::ImmId, Opcode>;

/// Helper struct for converting ir::BlockId to hhbc::Label.
pub(crate) struct Labeler {
    bid_to_label: BlockIdMap<hhbc::Label>,
    label_to_bid: HashMap<hhbc::Label, BlockId>,
    next_label_id: u32,
}

impl Labeler {
    pub(crate) fn new(func: &ir::Func) -> Self {
        let mut labeler = Self {
            bid_to_label: Default::default(),
            label_to_bid: Default::default(),
            next_label_id: 0,
        };
        labeler.prealloc_labels(func);
        labeler
    }

    // Go through the blocks and for any with well-known names assign those
    // names now. If we don't do this on init then we're liable to accidentally
    // use a name that we'll need later on.
    fn prealloc_labels(&mut self, func: &ir::Func) {
        fn pname_to_label(pname: &str) -> Option<hhbc::Label> {
            if let Some(label) = pname.strip_prefix('L') {
                label.parse::<u32>().ok().map(hhbc::Label)
            } else if let Some(label) = pname.strip_prefix("DV") {
                label.parse::<u32>().ok().map(hhbc::Label)
            } else {
                None
            }
        }

        for (bid, label) in func
            .repr
            .block_ids()
            .map(|bid| (bid, func.repr.block(bid)))
            .filter_map(|(bid, block)| block.pname_hint.as_ref().map(|pname| (bid, pname)))
            .filter_map(|(bid, pname)| pname_to_label(pname).map(|label| (bid, label)))
        {
            if let std::collections::hash_map::Entry::Vacant(e) = self.label_to_bid.entry(label) {
                self.bid_to_label.insert(bid, label);
                e.insert(bid);
                self.next_label_id = std::cmp::max(self.next_label_id, label.0 + 1);
            }
        }
    }

    fn get_bid(&self, bid: BlockId) -> Option<hhbc::Label> {
        self.bid_to_label.get(&bid).copied()
    }

    pub(crate) fn lookup_bid(&self, bid: BlockId) -> hhbc::Label {
        self.bid_to_label[&bid]
    }

    fn lookup_label(&self, label: hhbc::Label) -> Option<BlockId> {
        self.label_to_bid.get(&label).copied()
    }

    fn lookup_or_insert_bid(&mut self, bid: BlockId) -> hhbc::Label {
        *self.bid_to_label.entry(bid).or_insert_with(|| {
            let label = hhbc::Label(self.next_label_id);
            self.next_label_id += 1;
            self.label_to_bid.insert(label, bid);
            label
        })
    }
}

fn compute_block_entry_edges(func: &ir::Func) -> BlockIdMap<usize> {
    let mut edges = BlockIdMap::default();
    for (_, dv) in &func.repr.params {
        if let Some(dv) = dv {
            edges.entry(dv.init).and_modify(|e| *e += 1).or_insert(1);
        }
    }
    for bid in func.repr.block_ids() {
        edges.entry(bid).or_insert(0);
        for &edge in func.repr.edges(bid) {
            edges.entry(edge).and_modify(|e| *e += 1).or_insert(1);
        }
    }
    edges
}

pub(crate) struct InstrEmitter<'b> {
    // How many blocks jump to this one?
    block_entry_edges: BlockIdMap<usize>,
    func: &'b ir::Func,
    instrs: Vec<Instruct>,
    labeler: &'b mut Labeler,
    loc_id: ir::LocId,
    locals: HashMap<LocalId, hhbc::Local>,
    imm_to_opcode: &'b ImmToOpcode,
}

fn convert_indexes_to_bools(total_len: usize, indexes: Option<&[u32]>) -> Vec<bool> {
    let mut buf: Vec<bool> = Vec::with_capacity(total_len);
    if !indexes.is_none_or(|indexes| indexes.is_empty()) {
        buf.resize(total_len, false);
        if let Some(indexes) = indexes {
            for &idx in indexes.iter() {
                buf[idx as usize] = true;
            }
        }
    }
    buf
}

impl<'b> InstrEmitter<'b> {
    fn new(func: &'b ir::Func, labeler: &'b mut Labeler, imm_to_opcode: &'b ImmToOpcode) -> Self {
        let locals = Self::prealloc_locals(func);

        Self {
            block_entry_edges: compute_block_entry_edges(func),
            func,
            instrs: Vec::new(),
            labeler,
            loc_id: ir::LocId::NONE,
            locals,
            imm_to_opcode,
        }
    }

    fn prealloc_locals(func: &'b ir::Func) -> HashMap<LocalId, hhbc::Local> {
        let mut next_local_idx = 0;

        // The parameters are required to be the first named locals. We can't
        // control that.
        let mut locals: HashMap<LocalId, hhbc::Local> = func
            .repr
            .params
            .iter()
            .map(|(param, _)| {
                let name = LocalId::Named(param.name);
                let local = hhbc::Local::new(next_local_idx);
                next_local_idx += 1;
                (name, local)
            })
            .collect();

        trace!("Parameters end at {}", next_local_idx);

        // Now go through and collect the named and unnamed locals. Once we know
        // how many named locals there are we can fixup the unnamed ones.
        for instr in func.repr.body_instrs() {
            for lid in instr.locals() {
                match lid {
                    LocalId::Named(_) => {
                        locals.entry(*lid).or_insert_with(|| {
                            let local = hhbc::Local::new(next_local_idx);
                            next_local_idx += 1;
                            local
                        });
                    }
                    LocalId::Unnamed(idx) => {
                        locals
                            .entry(*lid)
                            .or_insert_with(|| hhbc::Local::new(idx.as_usize()));
                    }
                }
            }
        }

        trace!("Named locals end at {}", next_local_idx);

        // Now that we know the count of named locals we can update the unnamed
        // locals.
        for (k, v) in locals.iter_mut() {
            if matches!(k, LocalId::Unnamed(_)) {
                *v = hhbc::Local::new(v.index() + next_local_idx);
            }
        }

        locals
    }

    fn push_opcode(&mut self, opcode: Opcode) {
        self.instrs.push(Instruct::Opcode(opcode));
    }

    fn lookup_local(&mut self, lid: LocalId) -> hhbc::Local {
        if let Some(local) = self.locals.get(&lid) {
            *local
        } else {
            panic!("Unmapped LocalId {:?}", lid);
        }
    }

    fn emit_call_helper(&mut self, call: &instr::Call, async_eager_target: Option<hhbc::Label>) {
        let fcall_args = {
            use hhbc::FCallArgs;

            let mut num_args = call.args().len() as u32;

            num_args -= call.flags.contains(FCallArgsFlags::HasUnpack) as u32;
            num_args -= call.flags.contains(FCallArgsFlags::HasGenerics) as u32;

            let num_rets = call.num_rets;
            let inouts = convert_indexes_to_bools(num_args as usize, call.inouts.as_deref());
            let readonly = convert_indexes_to_bools(num_args as usize, call.readonly.as_deref());

            let async_eager_target = if let Some(label) = async_eager_target {
                label
            } else {
                hhbc::Label::INVALID
            };

            FCallArgs {
                flags: call.flags,
                async_eager_target,
                num_args,
                num_rets,
                inouts: inouts.into(),
                readonly: readonly.into(),
                context: call.context,
            }
        };
        let hint = BytesId::EMPTY;
        let instr = match &call.detail {
            ir::instr::CallDetail::FCallClsMethod { log } => {
                Opcode::FCallClsMethod(fcall_args, hint, *log)
            }
            ir::instr::CallDetail::FCallClsMethodD { clsid, method } => {
                Opcode::FCallClsMethodD(fcall_args, *clsid, *method)
            }
            ir::instr::CallDetail::FCallClsMethodM { method, log } => {
                Opcode::FCallClsMethodM(fcall_args, hint, *log, *method)
            }
            ir::instr::CallDetail::FCallClsMethodS { clsref } => {
                Opcode::FCallClsMethodS(fcall_args, hint, *clsref)
            }
            ir::instr::CallDetail::FCallClsMethodSD { method, clsref } => {
                Opcode::FCallClsMethodSD(fcall_args, hint, *clsref, *method)
            }
            ir::instr::CallDetail::FCallCtor => Opcode::FCallCtor(fcall_args, hint),
            ir::instr::CallDetail::FCallFunc => Opcode::FCallFunc(fcall_args),
            ir::instr::CallDetail::FCallFuncD { func } => Opcode::FCallFuncD(fcall_args, *func),
            ir::instr::CallDetail::FCallObjMethod { flavor } => {
                Opcode::FCallObjMethod(fcall_args, hint, *flavor)
            }
            ir::instr::CallDetail::FCallObjMethodD { flavor, method } => {
                Opcode::FCallObjMethodD(fcall_args, hint, *flavor, *method)
            }
        };
        self.push_opcode(instr);
    }

    fn emit_call(&mut self, call: &instr::Call) {
        self.emit_call_helper(call, None);
    }

    fn emit_call_async(&mut self, call: &instr::Call, targets: &[BlockId; 2]) {
        let label = self.labeler.lookup_or_insert_bid(targets[1]);
        self.emit_call_helper(call, Some(label));
        self.jump_to(targets[0]);
    }

    fn jump_to(&mut self, bid: BlockId) {
        if bid != BlockId::NONE {
            let label = self.labeler.lookup_or_insert_bid(bid);
            self.push_opcode(Opcode::Enter(label));
        }
    }

    fn convert_local_range(&mut self, slice: &[LocalId]) -> hhbc::LocalRange {
        let locals: Vec<hhbc::Local> = slice.iter().map(|lid| self.lookup_local(*lid)).collect();

        if let Some(&start) = locals.first() {
            let len = locals.len() as u32;

            // At this point pusher should have aligned stuff so that the
            // variables are all contiguous...
            locals.iter().reduce(|x, y| {
                assert!(
                    x.index() + 1 == y.index(),
                    "non-linear locals: [{}]",
                    locals
                        .iter()
                        .map(ToString::to_string)
                        .collect::<Vec<_>>()
                        .join(", ")
                );
                y
            });

            hhbc::LocalRange { start, len }
        } else {
            hhbc::LocalRange {
                start: hhbc::Local::INVALID,
                len: 0,
            }
        }
    }

    /// This is the emitter for HHBC opcodes. At this point the stack values
    /// have been pushed and we just need to emit the correct opcode for the
    /// pending instruction (along with its immediates).
    fn emit_hhbc(&mut self, hhbc: &instr::Hhbc) {
        use instr::Hhbc;
        let opcode = match *hhbc {
            Hhbc::AKExists(..) => Opcode::AKExists,
            Hhbc::Add(..) => Opcode::Add,
            Hhbc::AddElemC(..) => Opcode::AddElemC,
            Hhbc::AddNewElemC(..) => Opcode::AddNewElemC,
            Hhbc::ArrayIdx(..) => Opcode::ArrayIdx,
            Hhbc::ArrayMarkLegacy(..) => Opcode::ArrayMarkLegacy,
            Hhbc::ArrayUnmarkLegacy(..) => Opcode::ArrayUnmarkLegacy,
            Hhbc::Await(..) => Opcode::Await,
            Hhbc::AwaitAll(ref locals, _) => {
                let locals = self.convert_local_range(locals);
                Opcode::AwaitAll(locals)
            }
            Hhbc::AwaitLowPri(..) => Opcode::AwaitLowPri,
            Hhbc::BareThis(op, _) => Opcode::BareThis(op),
            Hhbc::BitAnd(..) => Opcode::BitAnd,
            Hhbc::BitNot(..) => Opcode::BitNot,
            Hhbc::BitOr(..) => Opcode::BitOr,
            Hhbc::BitXor(..) => Opcode::BitXor,
            Hhbc::CGetG(..) => Opcode::CGetG,
            Hhbc::CGetL(lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::CGetL(local)
            }
            Hhbc::CGetQuietL(lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::CGetQuietL(local)
            }
            Hhbc::CGetS(_, readonly, _) => Opcode::CGetS(readonly),
            Hhbc::CUGetL(lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::CUGetL(local)
            }
            Hhbc::CastBool(..) => Opcode::CastBool,
            Hhbc::CastDict(..) => Opcode::CastDict,
            Hhbc::CastDouble(..) => Opcode::CastDouble,
            Hhbc::CastInt(..) => Opcode::CastInt,
            Hhbc::CastKeyset(..) => Opcode::CastKeyset,
            Hhbc::CastString(..) => Opcode::CastString,
            Hhbc::CastVec(..) => Opcode::CastVec,
            Hhbc::CheckClsReifiedGenericMismatch(..) => Opcode::CheckClsReifiedGenericMismatch,
            Hhbc::CheckClsRGSoft(..) => Opcode::CheckClsRGSoft,
            Hhbc::ChainFaults(..) => Opcode::ChainFaults,
            Hhbc::CheckProp(prop, _) => Opcode::CheckProp(prop),
            Hhbc::CheckThis(_) => Opcode::CheckThis,
            Hhbc::ClassGetC(_, mode, _) => Opcode::ClassGetC(mode),
            Hhbc::ClassGetTS(_, _) => Opcode::ClassGetTS,
            Hhbc::ClassGetTSWithGenerics(_, _) => Opcode::ClassGetTSWithGenerics,
            Hhbc::ClassHasReifiedGenerics(_, _) => Opcode::ClassHasReifiedGenerics,
            Hhbc::ClassName(..) => Opcode::ClassName,
            Hhbc::Clone(..) => Opcode::Clone,
            Hhbc::ClsCns(_, id, _) => Opcode::ClsCns(id),
            Hhbc::ClsCnsD(id, clsid, _) => Opcode::ClsCnsD(id, clsid),
            Hhbc::ClsCnsL(_, lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::ClsCnsL(local)
            }
            Hhbc::Cmp(..) => Opcode::Cmp,
            Hhbc::CmpOp(_, op, _) => match op {
                instr::CmpOp::Eq => Opcode::Eq,
                instr::CmpOp::Gt => Opcode::Gt,
                instr::CmpOp::Gte => Opcode::Gte,
                instr::CmpOp::Lt => Opcode::Lt,
                instr::CmpOp::Lte => Opcode::Lte,
                instr::CmpOp::NSame => Opcode::NSame,
                instr::CmpOp::Neq => Opcode::Neq,
                instr::CmpOp::Same => Opcode::Same,
            },
            Hhbc::ColFromArray(_, kind, _) => Opcode::ColFromArray(kind),
            Hhbc::CombineAndResolveTypeStruct(ref vids, _) => {
                Opcode::CombineAndResolveTypeStruct(vids.len() as u32)
            }
            Hhbc::Concat(..) => Opcode::Concat,
            Hhbc::ConcatN(ref vids, _) => Opcode::ConcatN(vids.len() as u32),
            Hhbc::ConsumeL(lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::PushL(local)
            }
            Hhbc::ContCheck(kind, _) => Opcode::ContCheck(kind),
            Hhbc::ContCurrent(_) => Opcode::ContCurrent,
            Hhbc::ContEnter(..) => Opcode::ContEnter,
            Hhbc::ContGetReturn(_) => Opcode::ContGetReturn,
            Hhbc::ContKey(_) => Opcode::ContKey,
            Hhbc::ContRaise(..) => Opcode::ContRaise,
            Hhbc::ContValid(_) => Opcode::ContValid,
            Hhbc::CreateCl {
                ref operands,
                clsid,
                ..
            } => Opcode::CreateCl(operands.len() as u32, clsid),
            Hhbc::CreateCont(_) => Opcode::CreateCont,
            Hhbc::GetMemoAgnosticImplicitContext(..) => Opcode::GetMemoAgnosticImplicitContext,
            Hhbc::Div(..) => Opcode::Div,
            Hhbc::EnumClassLabelName(..) => Opcode::EnumClassLabelName,
            Hhbc::GetClsRGProp(..) => Opcode::GetClsRGProp,
            Hhbc::GetMemoKeyL(lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::GetMemoKeyL(local)
            }
            Hhbc::HasReifiedParent(_, _) => Opcode::HasReifiedParent,
            Hhbc::Idx(..) => Opcode::Idx,
            Hhbc::IncDecL(lid, op, _) => {
                let local = self.lookup_local(lid);
                Opcode::IncDecL(local, op)
            }
            Hhbc::IncDecS(_, op, _) => Opcode::IncDecS(op),
            Hhbc::IncludeEval(ref ie) => self.emit_include_eval(ie),
            Hhbc::InitProp(_, prop, op, _) => Opcode::InitProp(prop, op),
            Hhbc::InstanceOfD(_, clsid, _, _) => Opcode::InstanceOfD(clsid),
            Hhbc::IsLateBoundCls(_, _) => Opcode::IsLateBoundCls,
            Hhbc::IsTypeC(_, op, _) => Opcode::IsTypeC(op),
            Hhbc::IsTypeL(lid, op, _) => {
                let local = self.lookup_local(lid);
                Opcode::IsTypeL(local, op)
            }
            Hhbc::IsTypeStructC(_, op, kind, _) => Opcode::IsTypeStructC(op, kind),
            Hhbc::IssetG(_, _) => Opcode::IssetG,
            Hhbc::IssetL(lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::IssetL(local)
            }
            Hhbc::IssetS(_, _) => Opcode::IssetS,
            Hhbc::IterBase(..) => Opcode::IterBase,
            Hhbc::IterFree(iter_id, _) => Opcode::IterFree(iter_id),
            Hhbc::IterGetKey(iter_args, base_lid, _) => {
                let local = self.lookup_local(base_lid);
                Opcode::IterGetKey(iter_args, local)
            }
            Hhbc::IterGetValue(iter_args, base_lid, _) => {
                let local = self.lookup_local(base_lid);
                Opcode::IterGetValue(iter_args, local)
            }
            Hhbc::IterSetValue(_, iter_args, base_lid, _) => {
                let local = self.lookup_local(base_lid);
                Opcode::IterSetValue(iter_args, local)
            }
            Hhbc::LateBoundCls(_) => Opcode::LateBoundCls,
            Hhbc::LazyClassFromClass(_, _) => Opcode::LazyClassFromClass,
            Hhbc::LockObj(..) => Opcode::LockObj,
            Hhbc::MemoSet(_, ref locals, _) => {
                let locals = self.convert_local_range(locals);
                Opcode::MemoSet(locals)
            }
            Hhbc::MemoSetEager(_, ref locals, _) => {
                let locals = self.convert_local_range(locals);
                Opcode::MemoSetEager(locals)
            }
            Hhbc::Modulo(..) => Opcode::Mod,
            Hhbc::Mul(..) => Opcode::Mul,
            Hhbc::NewDictArray(hint, _) => Opcode::NewDictArray(hint),
            Hhbc::NewKeysetArray(ref operands, _) => Opcode::NewKeysetArray(operands.len() as u32),
            Hhbc::NewObj(_, _) => Opcode::NewObj,
            Hhbc::NewObjD(clsid, _) => Opcode::NewObjD(clsid),
            Hhbc::NewObjS(clsref, _) => Opcode::NewObjS(clsref),
            Hhbc::NewPair(..) => Opcode::NewPair,
            Hhbc::NewStructDict(ref keys, _, _) => Opcode::NewStructDict(keys.to_vec().into()),
            Hhbc::NewVec(ref vids, _) => Opcode::NewVec(vids.len() as u32),
            Hhbc::Not(..) => Opcode::Not,
            Hhbc::OODeclExists(_, kind, _) => Opcode::OODeclExists(kind),
            Hhbc::ParentCls(_) => Opcode::ParentCls,
            Hhbc::Pow(..) => Opcode::Pow,
            Hhbc::Print(..) => Opcode::Print,
            Hhbc::RaiseClassStringConversionNotice(..) => Opcode::RaiseClassStringConversionNotice,
            Hhbc::RecordReifiedGeneric(..) => Opcode::RecordReifiedGeneric,
            Hhbc::ReifiedInit(_, lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::ReifiedInit(local)
            }
            Hhbc::ResolveClass(clsid, _) => Opcode::ResolveClass(clsid),
            Hhbc::ResolveClsMethod(_, method, _) => Opcode::ResolveClsMethod(method),
            Hhbc::ResolveClsMethodD(clsid, method, _) => Opcode::ResolveClsMethodD(clsid, method),
            Hhbc::ResolveClsMethodS(clsref, method, _) => Opcode::ResolveClsMethodS(clsref, method),
            Hhbc::ResolveRClsMethod(_, method, _) => Opcode::ResolveRClsMethod(method),
            Hhbc::ResolveRClsMethodS(_, clsref, method, _) => {
                Opcode::ResolveRClsMethodS(clsref, method)
            }
            Hhbc::ResolveFunc(func, _) => Opcode::ResolveFunc(func),
            Hhbc::ResolveMethCaller(func, _) => Opcode::ResolveMethCaller(func),
            Hhbc::ResolveRClsMethodD(_, clsid, method, _) => {
                Opcode::ResolveRClsMethodD(clsid, method)
            }
            Hhbc::ResolveRFunc(_, func, _) => Opcode::ResolveRFunc(func),
            Hhbc::SelfCls(_) => Opcode::SelfCls,
            Hhbc::SetG(_, _) => Opcode::SetG,
            Hhbc::SetImplicitContextByValue(..) => Opcode::SetImplicitContextByValue,
            Hhbc::SetL(_, lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::SetL(local)
            }
            Hhbc::SetOpL(_, lid, op, _) => {
                let local = self.lookup_local(lid);
                Opcode::SetOpL(local, op)
            }
            Hhbc::SetOpS(_, op, _) => Opcode::SetOpS(op),
            Hhbc::SetS(_, readonly, _) => Opcode::SetS(readonly),
            Hhbc::Shl(..) => Opcode::Shl,
            Hhbc::Shr(..) => Opcode::Shr,
            Hhbc::Sub(..) => Opcode::Sub,
            Hhbc::This(_) => Opcode::This,
            Hhbc::ThrowNonExhaustiveSwitch(_) => Opcode::ThrowNonExhaustiveSwitch,
            Hhbc::UnsetG(_, _) => Opcode::UnsetG,
            Hhbc::UnsetL(lid, _) => {
                let local = self.lookup_local(lid);
                Opcode::UnsetL(local)
            }
            Hhbc::VerifyOutType(_, pid, _) => {
                let local = self.lookup_local(pid);
                Opcode::VerifyOutType(local)
            }
            Hhbc::VerifyParamType(_, pid, _) => {
                let local = self.lookup_local(pid);
                Opcode::VerifyParamType(local)
            }
            Hhbc::VerifyParamTypeTS(_, pid, _) => {
                let local = self.lookup_local(pid);
                Opcode::VerifyParamTypeTS(local)
            }
            Hhbc::VerifyRetTypeC(..) => Opcode::VerifyRetTypeC,
            Hhbc::VerifyRetTypeTS(..) => Opcode::VerifyRetTypeTS,
            Hhbc::VerifyTypeTS(..) => Opcode::VerifyTypeTS,
            Hhbc::WHResult(..) => Opcode::WHResult,
            Hhbc::Yield(..) => Opcode::Yield,
            Hhbc::YieldK(..) => Opcode::YieldK,
        };
        self.push_opcode(opcode);
    }

    fn emit_instr(&mut self, iid: ir::InstrId, instr: &ir::Instr) {
        self.emit_loc(instr.loc_id());

        trace!("    Instr: {}", ir::print::FmtInstr(self.func, iid));

        use ir::Instr;
        match instr {
            Instr::Call(call) => self.emit_call(call),
            Instr::Hhbc(hhbc) => self.emit_hhbc(hhbc),
            Instr::MemberOp(op) => self.emit_member_op(op),
            Instr::Special(special) => self.emit_special(special),
            Instr::Terminator(terminator) => self.emit_terminator(terminator),
        }
    }

    fn emit_constant(&mut self, imm: ir::ImmId) {
        self.push_opcode(self.imm_to_opcode[imm].clone());
    }

    fn emit_loc(&mut self, loc_id: ir::LocId) {
        if self.loc_id != loc_id {
            if let Some(loc) = self.func.repr.get_loc(loc_id) {
                self.loc_id = loc_id;
                self.instrs
                    .push(Instruct::Pseudo(Pseudo::SrcLoc(hhbc::SrcLoc {
                        line_begin: loc.line_begin,
                        col_begin: loc.col_begin,
                        line_end: loc.line_end,
                        col_end: loc.col_end,
                    })));
            }
        }
    }

    fn emit_include_eval(&mut self, ie: &instr::IncludeEval) -> Opcode {
        use instr::IncludeKind;
        match ie.kind {
            IncludeKind::Eval => Opcode::Eval,
            IncludeKind::Include => Opcode::Incl,
            IncludeKind::IncludeOnce => Opcode::InclOnce,
            IncludeKind::Require => Opcode::Req,
            IncludeKind::RequireOnce => Opcode::ReqOnce,
            IncludeKind::RequireOnceDoc => Opcode::ReqDoc,
        }
    }

    /// Emitter for a MemberOp instruction. In IR the MemberOp is a single
    /// instruction but that maps to multiple HHBC instructions (a BaseOp, zero
    /// or more Dim Instructs, and a FinalOp).
    #[allow(clippy::todo)]
    fn emit_member_op(&mut self, op: &instr::MemberOp) {
        use instr::BaseOp;
        use instr::FinalOp;

        let mut stack_index = op.operands.len() as u32;
        let mut locals = op.locals.iter().copied();

        let may_mutate_stack_base = member_op_may_mutate_stack_base(op);

        match op.base_op {
            BaseOp::BaseC { mode, .. } => {
                stack_index -= 1;
                self.push_opcode(Opcode::BaseC(stack_index, mode));
            }
            BaseOp::BaseH { .. } => {
                self.push_opcode(Opcode::BaseH);
            }
            BaseOp::BaseL { mode, readonly, .. } => {
                let lid = locals.next().unwrap();
                let local = self.lookup_local(lid);
                self.push_opcode(Opcode::BaseL(local, mode, readonly));
            }
            BaseOp::BaseSC { mode, readonly, .. } => {
                stack_index -= 2;
                self.push_opcode(Opcode::BaseSC(stack_index + 1, stack_index, mode, readonly));
            }
            BaseOp::BaseST { .. } => {
                todo!("BaseST not allowed in HHBC");
            }
        }

        for interm_op in op.intermediate_ops.iter() {
            self.emit_loc(interm_op.loc_id());
            let key = self.convert_member_key(
                &mut stack_index,
                &mut locals,
                &interm_op.key,
                interm_op.readonly,
            );
            self.push_opcode(Opcode::Dim(interm_op.mode, key));
        }

        let num = op.operands.len() as u32 - may_mutate_stack_base as u32;

        match op.final_op {
            FinalOp::IncDecM {
                inc_dec_op,
                ref key,
                readonly,
                loc,
                ..
            } => {
                let key = self.convert_member_key(&mut stack_index, &mut locals, key, readonly);
                self.emit_loc(loc);
                self.push_opcode(Opcode::IncDecM(num, inc_dec_op, key));
            }
            FinalOp::QueryM {
                query_m_op,
                ref key,
                readonly,
                loc,
                ..
            } => {
                let key = self.convert_member_key(&mut stack_index, &mut locals, key, readonly);
                self.emit_loc(loc);
                self.push_opcode(Opcode::QueryM(num, query_m_op, key));
            }
            FinalOp::SetM {
                ref key,
                readonly,
                loc,
                ..
            } => {
                let key = self.convert_member_key(&mut stack_index, &mut locals, key, readonly);
                self.emit_loc(loc);
                // `data.operands()` includes the stored value but num does not.
                self.push_opcode(Opcode::SetM(num - 1, key));
                // Extra stack slot for the value.
                stack_index -= 1;
            }
            FinalOp::SetOpM {
                ref key,
                readonly,
                set_op_op,
                loc,
                ..
            } => {
                let key = self.convert_member_key(&mut stack_index, &mut locals, key, readonly);
                self.emit_loc(loc);
                // `data.operands()` includes the stored value but num does not.
                self.push_opcode(Opcode::SetOpM(num - 1, set_op_op, key));
                // Extra stack slot for the value.
                stack_index -= 1;
            }
            FinalOp::SetRangeM {
                sz,
                set_range_op,
                loc,
                ..
            } => {
                self.emit_loc(loc);
                // `data.operands()` includes the params but num does not.
                self.push_opcode(Opcode::SetRangeM(num - 3, sz, set_range_op));
                // Extra stack slot for the values.
                stack_index -= 3;
            }
            FinalOp::UnsetM {
                ref key,
                readonly,
                loc,
                ..
            } => {
                let key = self.convert_member_key(&mut stack_index, &mut locals, key, readonly);
                self.emit_loc(loc);
                self.push_opcode(Opcode::UnsetM(num, key));
            }
        }

        assert_eq!(stack_index as usize, 0);
    }

    fn convert_member_key(
        &mut self,
        stack_index: &mut u32,
        locals: &mut impl Iterator<Item = LocalId>,
        key: &instr::MemberKey,
        readonly: ir::ReadonlyOp,
    ) -> hhbc::MemberKey {
        match *key {
            instr::MemberKey::EC => {
                *stack_index -= 1;
                hhbc::MemberKey::EC(*stack_index, readonly)
            }
            instr::MemberKey::EI(imm) => hhbc::MemberKey::EI(imm, readonly),
            instr::MemberKey::EL => {
                let lid = locals.next().unwrap();
                let local = self.lookup_local(lid);
                hhbc::MemberKey::EL(local, readonly)
            }
            instr::MemberKey::ET(name) => hhbc::MemberKey::ET(name, readonly),
            instr::MemberKey::PC => {
                *stack_index -= 1;
                hhbc::MemberKey::PC(*stack_index, readonly)
            }
            instr::MemberKey::PL => {
                let lid = locals.next().unwrap();
                let local = self.lookup_local(lid);
                hhbc::MemberKey::PL(local, readonly)
            }
            instr::MemberKey::PT(name) => hhbc::MemberKey::PT(name, readonly),
            instr::MemberKey::QT(name) => hhbc::MemberKey::QT(name, readonly),
            instr::MemberKey::W => hhbc::MemberKey::W,
        }
    }

    fn emit_special(&mut self, special: &instr::Special) {
        use instr::Special;
        match special {
            Special::IrToBc(ir_to_bc) => self.emit_ir_to_bc(ir_to_bc),
            Special::Param => {}
            Special::Select(..) => {
                // Select is entirely handled during the push/pop phase.
            }
            Special::Copy(_) | Special::Textual(_) | Special::Tmp(..) | Special::Tombstone => {
                panic!("shouldn't be trying to emit {special:?}")
            }
        }
    }

    fn emit_ir_to_bc(&mut self, ir_to_bc: &instr::IrToBc) {
        use ir::FullInstrId;
        match ir_to_bc {
            IrToBc::PopC => {
                self.push_opcode(Opcode::PopC);
            }
            IrToBc::PopL(lid) => {
                let local = self.lookup_local(*lid);
                self.push_opcode(Opcode::PopL(local));
            }
            IrToBc::PushL(lid) => {
                let local = self.lookup_local(*lid);
                self.push_opcode(Opcode::PushL(local));
            }
            IrToBc::PushConstant(vid) => match vid.full() {
                FullInstrId::Imm(cid) => self.emit_constant(cid),
                FullInstrId::Instr(_) | FullInstrId::None => panic!("Malformed PushConstant!"),
            },
            IrToBc::PushUninit => self.push_opcode(Opcode::NullUninit),
            IrToBc::UnsetL(lid) => {
                let local = self.lookup_local(*lid);
                self.push_opcode(Opcode::UnsetL(local));
            }
        }
    }

    fn emit_terminator(&mut self, terminator: &instr::Terminator) {
        use instr::Terminator;
        match *terminator {
            Terminator::CallAsync(ref call, ref targets) => self.emit_call_async(call, targets),
            Terminator::Enter(bid, _) => {
                let label = self.labeler.lookup_or_insert_bid(bid);
                self.push_opcode(Opcode::Enter(label));
            }
            Terminator::Exit(..) => {
                self.push_opcode(Opcode::Exit);
            }
            Terminator::Fatal(_, op, _) => {
                self.push_opcode(Opcode::Fatal(op));
            }
            Terminator::IterInit(ref ir_args) => {
                let args = hhbc::IterArgs {
                    iter_id: ir_args.iter_id,
                    flags: ir_args.flags,
                };
                let base_id = self.lookup_local(ir_args.base_lid);
                let label = self.labeler.lookup_or_insert_bid(ir_args.done_bid());
                self.push_opcode(Opcode::IterInit(args, base_id, label));
                self.jump_to(ir_args.next_bid());
            }
            Terminator::IterNext(ref ir_args) => {
                let args = hhbc::IterArgs {
                    iter_id: ir_args.iter_id,
                    flags: ir_args.flags,
                };
                let base_id = self.lookup_local(ir_args.base_lid);
                let label = self.labeler.lookup_or_insert_bid(ir_args.done_bid());
                self.push_opcode(Opcode::IterNext(args, base_id, label));
                self.jump_to(ir_args.next_bid());
            }
            Terminator::Jmp(bid, _) | Terminator::JmpArgs(bid, _, _) => {
                let label = self.labeler.lookup_or_insert_bid(bid);
                self.push_opcode(Opcode::Jmp(label));
            }
            Terminator::JmpOp {
                cond: _,
                ref pred,
                targets,
                loc: _,
            } => {
                use ir::instr::Predicate;
                let t_label = self.labeler.lookup_or_insert_bid(targets[0]);
                let i = match pred {
                    Predicate::NonZero => Opcode::JmpNZ(t_label),
                    Predicate::Zero => Opcode::JmpZ(t_label),
                };
                self.push_opcode(i);
                self.jump_to(targets[1]);
            }
            Terminator::MemoGet(ref get) => {
                let locals = self.convert_local_range(&get.locals);
                let else_edge = self.labeler.lookup_or_insert_bid(get.no_value_edge());
                self.push_opcode(Opcode::MemoGet(else_edge, locals));
                self.jump_to(get.value_edge());
            }
            Terminator::MemoGetEager(ref get) => {
                let locals = self.convert_local_range(&get.locals);
                let no_value = self.labeler.lookup_or_insert_bid(get.no_value_edge());
                let suspended = self.labeler.lookup_or_insert_bid(get.suspended_edge());
                self.push_opcode(Opcode::MemoGetEager(
                    [no_value, suspended],
                    Dummy::DEFAULT,
                    locals,
                ));
                self.jump_to(get.eager_edge());
            }
            Terminator::NativeImpl(_) => {
                self.push_opcode(Opcode::NativeImpl);
            }
            Terminator::Ret(_, _) => {
                self.push_opcode(Opcode::RetC);
            }
            Terminator::RetCSuspended(_, _) => {
                self.push_opcode(Opcode::RetCSuspended);
            }
            Terminator::RetM(ref vids, _) => {
                self.push_opcode(Opcode::RetM(vids.len() as u32));
            }
            Terminator::Switch {
                bounded,
                base,
                ref targets,
                ..
            } => {
                let targets = Vec::from_iter(
                    targets
                        .iter()
                        .copied()
                        .map(|bid| self.labeler.lookup_or_insert_bid(bid)),
                );
                self.push_opcode(Opcode::Switch(bounded, base, targets.into()));
            }
            Terminator::SSwitch {
                ref cases,
                ref targets,
                ..
            } => {
                let cases = cases.to_vec().into();
                let targets = Vec::from_iter(
                    targets
                        .iter()
                        .map(|bid| self.labeler.lookup_or_insert_bid(*bid)),
                )
                .into();

                self.push_opcode(Opcode::SSwitch(cases, targets));
            }
            Terminator::Throw(_src, _) => {
                self.push_opcode(Opcode::Throw);
            }
            Terminator::ThrowAsTypeStructException(_, kind, _) => {
                self.push_opcode(Opcode::ThrowAsTypeStructException(kind));
            }
            Terminator::Unreachable => panic!("shouldn't be trying to emit unreachable"),
        };
    }

    fn scan_for_run_on_jump(&mut self, bid: BlockId) -> bool {
        // If the last emitted instr was a jump to this block then it can be elided.
        if self.instrs.is_empty() {
            return false;
        }

        let mut idx = self.instrs.len();
        while idx > 0 {
            idx -= 1;
            let instr = self.instrs.get(idx).unwrap();
            match *instr {
                Instruct::Pseudo(Pseudo::TryCatchBegin) => {
                    // We can just fall through a TryCatchBegin.
                    continue;
                }
                Instruct::Opcode(Opcode::Enter(label)) | Instruct::Opcode(Opcode::Jmp(label)) => {
                    // The last "interesting" instr was a jump.
                    if let Some(target) = self.labeler.lookup_label(label) {
                        if bid == target {
                            self.instrs.remove(idx);
                            return true;
                        }
                    }
                }
                _ => {}
            }
            break;
        }

        false
    }

    fn convert_block(&mut self, bid: BlockId) {
        let block = self.func.repr.block(bid);
        trace!(
            "Block {}{}{}",
            bid,
            self.labeler.get_bid(bid).map_or_else(String::new, |i| {
                let is_dv = false; // FIXME
                let prefix = if is_dv { "DV" } else { "L" };
                format!(" ({}{})", prefix, i.0)
            }),
            if block.params.is_empty() {
                "".to_owned()
            } else {
                format!(
                    ", args: [{}]",
                    FmtSep::comma(block.params.iter(), |w, vid| FmtRawVid(
                        ir::ValueId::from_instr(*vid)
                    )
                    .fmt(w))
                )
            }
        );

        let run_on = self.scan_for_run_on_jump(bid);

        // Only emit the label for this block if someone actually calls it.
        if self.block_entry_edges[&bid] > (run_on as usize) {
            let label = self.labeler.lookup_or_insert_bid(bid);
            self.instrs.push(Instruct::Pseudo(Pseudo::Label(label)));
        } else {
            trace!("  skipped label (run_on = {run_on})");
        }

        for (iid, instr) in block.iids().map(|iid| (iid, self.func.repr.instr(iid))) {
            self.emit_instr(iid, instr);
        }
    }

    fn get_next_block(&self, deque: &mut VecDeque<BlockIdOrExFrame>) -> Option<BlockIdOrExFrame> {
        // If the last bytecode emitted was an unconditional jump (JmpNS) and
        // its target is an unnamed, unemitted block then prefer that block
        // next.
        match self.instrs.last() {
            Some(&Instruct::Opcode(Opcode::Enter(label))) => {
                // The last instr WAS a jump.
                if let Some(bid) = self.labeler.lookup_label(label) {
                    if let Some(idx) = deque.iter().position(|i| i.bid() == bid) {
                        // And we haven't yet emitted it.
                        return deque.remove(idx);
                    }
                }
            }
            _ => {}
        }

        deque.pop_front()
    }

    fn convert_frame_vec(&mut self, vec: Vec<BlockIdOrExFrame>) {
        let mut deque: VecDeque<BlockIdOrExFrame> = vec.into();
        while let Some(item) = self.get_next_block(&mut deque) {
            match item {
                BlockIdOrExFrame::Block(bid) => {
                    self.convert_block(bid);
                }
                BlockIdOrExFrame::Frame(frame) => {
                    self.convert_frame(frame);
                }
            }
        }
    }

    fn convert_frame(&mut self, frame: ExFrame) {
        self.instrs.push(Instruct::Pseudo(Pseudo::TryCatchBegin));
        self.convert_frame_vec(frame.try_bids);
        self.instrs.push(Instruct::Pseudo(Pseudo::TryCatchMiddle));
        self.convert_frame_vec(frame.catch_bids);
        self.instrs.push(Instruct::Pseudo(Pseudo::TryCatchEnd));
    }
}

/// Determine if a MemberOp can mutate its Base as a stack entry. This is
/// important to know because if it mutates its base then we need to leave the
/// base on the stack instead of popping it off when the member_op is finished.
fn member_op_may_mutate_stack_base(member_op: &instr::MemberOp) -> bool {
    use instr::BaseOp;
    use instr::FinalOp;
    let write_op = match member_op.final_op {
        FinalOp::SetRangeM { .. } => true,
        FinalOp::UnsetM { .. } => true,
        FinalOp::IncDecM { .. } => true,
        FinalOp::QueryM { .. } => false,
        FinalOp::SetM { .. } => true,
        FinalOp::SetOpM { .. } => true,
    };

    let base_key_is_element_access = member_op.intermediate_ops.first().map_or_else(
        || {
            member_op
                .final_op
                .key()
                .is_none_or(|k| k.is_element_access())
        },
        |dim| dim.key.is_element_access(),
    );

    match member_op.base_op {
        BaseOp::BaseC { .. } => base_key_is_element_access && write_op,
        BaseOp::BaseH { .. }
        | BaseOp::BaseL { .. }
        | BaseOp::BaseSC { .. }
        | BaseOp::BaseST { .. } => false,
    }
}

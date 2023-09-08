// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::instr;
use ir::instr::IrToBc;
use ir::instr::Special;
use ir::Instr;

pub(crate) trait PushCount<'a> {
    /// How many values are pushed onto the stack?
    fn push_count(&self) -> usize;
}

impl<'a> PushCount<'a> for Instr {
    fn push_count(&self) -> usize {
        match self {
            // We shouldn't be asking for push count on some types.
            Instr::Special(Special::Tmp(_)) | Instr::Terminator(_) => unreachable!(),
            // Other types are complex and have to compute their push count
            // themselves.
            Instr::Call(call) => call.num_rets as usize,
            Instr::Hhbc(hhbc) => hhbc.push_count(),
            Instr::MemberOp(op) => op.num_values(),
            // --- 0 pushed values
            Instr::Special(
                Special::Copy(_)
                | Special::Textual(_)
                | Special::IrToBc(
                    IrToBc::PopC
                    | IrToBc::PopL(_)
                    | IrToBc::PushL(_)
                    | IrToBc::PushConstant(..)
                    | IrToBc::PushUninit
                    | IrToBc::UnsetL(_),
                )
                | Special::Select(..)
                | Special::Tombstone,
            ) => 0,
            // --- 1 pushed value
            Instr::Special(Special::Param) => 1,
        }
    }
}

impl<'a> PushCount<'a> for instr::Hhbc {
    fn push_count(&self) -> usize {
        use instr::Hhbc;
        match self {
            // --- 0 pushed values
            Hhbc::CheckClsReifiedGenericMismatch(..)
            | Hhbc::CheckClsRGSoft(..)
            | Hhbc::CheckThis(_)
            | Hhbc::ContCheck(..)
            | Hhbc::InitProp(..)
            | Hhbc::IterFree(..)
            | Hhbc::RaiseClassStringConversionWarning(..)
            | Hhbc::Silence(..)
            | Hhbc::ThrowNonExhaustiveSwitch(_)
            | Hhbc::UnsetG(..)
            | Hhbc::UnsetL(..)
            | Hhbc::VerifyImplicitContextState(_)
            | Hhbc::VerifyParamTypeTS(..) => 0,

            // --- 1 pushed value
            Hhbc::AKExists(..)
            | Hhbc::Add(..)
            | Hhbc::AddElemC(..)
            | Hhbc::AddNewElemC(..)
            | Hhbc::ArrayIdx(..)
            | Hhbc::ArrayMarkLegacy(..)
            | Hhbc::ArrayUnmarkLegacy(..)
            | Hhbc::Await(..)
            | Hhbc::AwaitAll(..)
            | Hhbc::BareThis(..)
            | Hhbc::BitAnd(..)
            | Hhbc::BitNot(..)
            | Hhbc::BitOr(..)
            | Hhbc::BitXor(..)
            | Hhbc::CGetG(..)
            | Hhbc::CGetL(..)
            | Hhbc::CGetQuietL(..)
            | Hhbc::CGetS(..)
            | Hhbc::CUGetL(..)
            | Hhbc::CastBool(..)
            | Hhbc::CastDict(..)
            | Hhbc::CastDouble(..)
            | Hhbc::CastInt(..)
            | Hhbc::CastKeyset(..)
            | Hhbc::CastString(..)
            | Hhbc::CastVec(..)
            | Hhbc::ChainFaults(..)
            | Hhbc::CheckProp(..)
            | Hhbc::ClassGetC(..)
            | Hhbc::ClassHasReifiedGenerics(..)
            | Hhbc::ClassName(..)
            | Hhbc::Clone(..)
            | Hhbc::ClsCns(..)
            | Hhbc::ClsCnsD(..)
            | Hhbc::ClsCnsL(..)
            | Hhbc::Cmp(..)
            | Hhbc::CmpOp(..)
            | Hhbc::ColFromArray(..)
            | Hhbc::CombineAndResolveTypeStruct(..)
            | Hhbc::Concat(..)
            | Hhbc::ConcatN(..)
            | Hhbc::ConsumeL(..)
            | Hhbc::ContCurrent(_)
            | Hhbc::ContEnter(..)
            | Hhbc::ContGetReturn(_)
            | Hhbc::ContKey(_)
            | Hhbc::ContRaise(..)
            | Hhbc::ContValid(_)
            | Hhbc::CreateCl { .. }
            | Hhbc::CreateCont(..)
            | Hhbc::CreateSpecialImplicitContext(..)
            | Hhbc::Div(..)
            | Hhbc::EnumClassLabelName(..)
            | Hhbc::GetClsRGProp(..)
            | Hhbc::GetMemoKeyL(..)
            | Hhbc::HasReifiedParent(..)
            | Hhbc::Idx(..)
            | Hhbc::IncDecL(..)
            | Hhbc::IncDecS(..)
            | Hhbc::IncludeEval(_)
            | Hhbc::InstanceOfD(..)
            | Hhbc::IsLateBoundCls(..)
            | Hhbc::IsTypeC(..)
            | Hhbc::IsTypeL(..)
            | Hhbc::IsTypeStructC(..)
            | Hhbc::IssetG(..)
            | Hhbc::IssetL(..)
            | Hhbc::IssetS(..)
            | Hhbc::LateBoundCls(_)
            | Hhbc::LazyClass(..)
            | Hhbc::LazyClassFromClass(..)
            | Hhbc::LockObj { .. }
            | Hhbc::MemoSet(..)
            | Hhbc::MemoSetEager(..)
            | Hhbc::Modulo(..)
            | Hhbc::Mul(..)
            | Hhbc::NewDictArray(..)
            | Hhbc::NewKeysetArray(..)
            | Hhbc::NewObj(..)
            | Hhbc::NewObjD(..)
            | Hhbc::NewObjS(..)
            | Hhbc::NewPair(..)
            | Hhbc::NewStructDict(..)
            | Hhbc::NewVec(..)
            | Hhbc::Not(..)
            | Hhbc::OODeclExists(..)
            | Hhbc::ParentCls(_)
            | Hhbc::Pow(..)
            | Hhbc::Print(..)
            | Hhbc::RecordReifiedGeneric(..)
            | Hhbc::ResolveClass(..)
            | Hhbc::ResolveClsMethod(..)
            | Hhbc::ResolveClsMethodD(..)
            | Hhbc::ResolveClsMethodS(..)
            | Hhbc::ResolveFunc(..)
            | Hhbc::ResolveMethCaller(..)
            | Hhbc::ResolveRClsMethod(..)
            | Hhbc::ResolveRClsMethodD(..)
            | Hhbc::ResolveRClsMethodS(..)
            | Hhbc::ResolveRFunc(..)
            | Hhbc::SelfCls(_)
            | Hhbc::SetG(..)
            | Hhbc::SetImplicitContextByValue(..)
            | Hhbc::SetL(..)
            | Hhbc::SetOpG(..)
            | Hhbc::SetOpL(..)
            | Hhbc::SetOpS(..)
            | Hhbc::SetS(..)
            | Hhbc::Shl(..)
            | Hhbc::Shr(..)
            | Hhbc::Sub(..)
            | Hhbc::This(_)
            | Hhbc::VerifyOutType(..)
            | Hhbc::VerifyParamType(..)
            | Hhbc::VerifyRetTypeC(..)
            | Hhbc::VerifyRetTypeTS(..)
            | Hhbc::WHResult(..)
            | Hhbc::Yield(..)
            | Hhbc::YieldK(..) => 1,

            // --- 2 pushed values
            Hhbc::ClassGetTS(..) => 2,
        }
    }
}

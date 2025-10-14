use hhbc::Local;
use hhbc::LocalRange;
use hhbc::Opcode;

use crate::node::NodeInstr;

pub(crate) enum LocalInfo {
    None,
    Read(Local),
    Write(Local),
    Mutate(Local),
    ReadRange(LocalRange),
}

impl LocalInfo {
    pub(crate) fn is_none(&self) -> bool {
        matches!(self, LocalInfo::None)
    }

    pub(crate) fn locals(&self) -> LocalRange {
        match self {
            LocalInfo::None => LocalRange::EMPTY,
            LocalInfo::Read(local) | LocalInfo::Write(local) | LocalInfo::Mutate(local) => {
                LocalRange::from_local(*local)
            }
            LocalInfo::ReadRange(range) => *range,
        }
    }

    pub(crate) fn for_opcode(opcode: &Opcode) -> LocalInfo {
        match opcode {
            Opcode::AwaitAll(range)
            | Opcode::MemoGet(_, range)
            | Opcode::MemoGetEager(_, _, range)
            | Opcode::MemoSet(range)
            | Opcode::MemoSetEager(range) => LocalInfo::ReadRange(*range),

            Opcode::AssertRATL(local, _)
            | Opcode::BaseL(local, _, _)
            | Opcode::CGetL(local)
            | Opcode::CGetL2(local)
            | Opcode::CGetQuietL(local)
            | Opcode::CUGetL(local)
            | Opcode::ClsCnsL(local)
            | Opcode::GetMemoKeyL(local)
            | Opcode::IsTypeL(local, _)
            | Opcode::IsUnsetL(local)
            | Opcode::IssetL(local)
            | Opcode::IterGetKey(_, local)
            | Opcode::IterGetValue(_, local)
            | Opcode::IterInit(_, local, _)
            | Opcode::IterNext(_, local, _)
            | Opcode::ReifiedInit(local) => LocalInfo::Read(*local),

            Opcode::PopL(local) | Opcode::SetL(local) | Opcode::UnsetL(local) => {
                LocalInfo::Write(*local)
            }

            Opcode::SetOpL(local, _)
            | Opcode::IncDecL(local, _)
            | Opcode::PushL(local)
            | Opcode::IterSetValue(_, local) => LocalInfo::Mutate(*local),

            Opcode::AKExists
            | Opcode::Add
            | Opcode::AddElemC
            | Opcode::AddNewElemC
            | Opcode::ArrayIdx
            | Opcode::ArrayMarkLegacy
            | Opcode::ArrayUnmarkLegacy
            | Opcode::AssertRATStk(..)
            | Opcode::Await
            | Opcode::AwaitLowPri
            | Opcode::BareThis(..)
            | Opcode::BaseC(..)
            | Opcode::BaseH
            | Opcode::BaseSC(..)
            | Opcode::BitAnd
            | Opcode::BitNot
            | Opcode::BitOr
            | Opcode::BitXor
            | Opcode::CGetCUNop
            | Opcode::CGetG
            | Opcode::CGetS(..)
            | Opcode::CastBool
            | Opcode::CastDict
            | Opcode::CastDouble
            | Opcode::CastInt
            | Opcode::CastKeyset
            | Opcode::CastString
            | Opcode::CastVec
            | Opcode::ChainFaults
            | Opcode::CheckProp(..)
            | Opcode::CheckClsReifiedGenericMismatch
            | Opcode::CheckClsRGSoft
            | Opcode::CheckThis
            | Opcode::ClassGetC(..)
            | Opcode::ClassGetTS
            | Opcode::ClassGetTSWithGenerics
            | Opcode::ClassHasReifiedGenerics
            | Opcode::ClassName
            | Opcode::Clone
            | Opcode::ClsCns(..)
            | Opcode::ClsCnsD(..)
            | Opcode::Cmp
            | Opcode::CnsE(..)
            | Opcode::ColFromArray(..)
            | Opcode::CombineAndResolveTypeStruct(..)
            | Opcode::Concat
            | Opcode::ConcatN(..)
            | Opcode::ContCheck(..)
            | Opcode::ContCurrent
            | Opcode::ContEnter
            | Opcode::ContGetReturn
            | Opcode::ContKey
            | Opcode::ContRaise
            | Opcode::ContValid
            | Opcode::CreateCl(..)
            | Opcode::CreateCont
            | Opcode::GetMemoAgnosticImplicitContext
            | Opcode::DblAsBits
            | Opcode::Dict(..)
            | Opcode::Dim(..)
            | Opcode::Dir
            | Opcode::Div
            | Opcode::Double(..)
            | Opcode::Dup
            | Opcode::Enter(..)
            | Opcode::EnumClassLabel(..)
            | Opcode::EnumClassLabelName
            | Opcode::Eq
            | Opcode::Eval
            | Opcode::Exit
            | Opcode::FCallClsMethod(..)
            | Opcode::FCallClsMethodD(..)
            | Opcode::FCallClsMethodM(..)
            | Opcode::FCallClsMethodS(..)
            | Opcode::FCallClsMethodSD(..)
            | Opcode::FCallCtor(..)
            | Opcode::FCallFunc(..)
            | Opcode::FCallFuncD(..)
            | Opcode::FCallObjMethod(..)
            | Opcode::FCallObjMethodD(..)
            | Opcode::False
            | Opcode::Fatal(..)
            | Opcode::File
            | Opcode::FuncCred
            | Opcode::GetClsRGProp
            | Opcode::Gt
            | Opcode::Gte
            | Opcode::HasReifiedParent
            | Opcode::Idx
            | Opcode::IncDecM(..)
            | Opcode::IncDecS(..)
            | Opcode::Incl
            | Opcode::InclOnce
            | Opcode::InitProp(..)
            | Opcode::InstanceOf
            | Opcode::InstanceOfD(..)
            | Opcode::Int(..)
            | Opcode::IsLateBoundCls
            | Opcode::IsTypeC(..)
            | Opcode::IsTypeStructC(..)
            | Opcode::IssetG
            | Opcode::IssetS
            | Opcode::IterBase
            | Opcode::IterFree(..)
            | Opcode::Jmp(..)
            | Opcode::JmpNZ(..)
            | Opcode::JmpZ(..)
            | Opcode::Keyset(..)
            | Opcode::LateBoundCls
            | Opcode::LazyClass(..)
            | Opcode::LockObj
            | Opcode::Lt
            | Opcode::Lte
            | Opcode::Method
            | Opcode::Mod
            | Opcode::Mul
            | Opcode::NSame
            | Opcode::NativeImpl
            | Opcode::Neq
            | Opcode::NewCol(..)
            | Opcode::NewDictArray(..)
            | Opcode::NewKeysetArray(..)
            | Opcode::NewObj
            | Opcode::NewObjD(..)
            | Opcode::NewObjS(..)
            | Opcode::NewPair
            | Opcode::NewStructDict(..)
            | Opcode::NewVec(..)
            | Opcode::Nop
            | Opcode::Not
            | Opcode::Null
            | Opcode::NullUninit
            | Opcode::OODeclExists(..)
            | Opcode::ParentCls
            | Opcode::PopC
            | Opcode::PopG
            | Opcode::PopU
            | Opcode::PopU2
            | Opcode::Pow
            | Opcode::Print
            | Opcode::QueryM(..)
            | Opcode::RaiseClassStringConversionNotice
            | Opcode::RecordReifiedGeneric
            | Opcode::Req
            | Opcode::ReqDoc
            | Opcode::ReqOnce
            | Opcode::ResolveClass(..)
            | Opcode::ResolveClsMethod(..)
            | Opcode::ResolveClsMethodD(..)
            | Opcode::ResolveClsMethodS(..)
            | Opcode::ResolveFunc(..)
            | Opcode::ResolveMethCaller(..)
            | Opcode::ResolveRClsMethod(..)
            | Opcode::ResolveRClsMethodD(..)
            | Opcode::ResolveRClsMethodS(..)
            | Opcode::ResolveRFunc(..)
            | Opcode::RetC
            | Opcode::RetCSuspended
            | Opcode::RetM(..)
            | Opcode::SSwitch { .. }
            | Opcode::Same
            | Opcode::Select
            | Opcode::SelfCls
            | Opcode::SetImplicitContextByValue
            | Opcode::SetM(..)
            | Opcode::SetOpM(..)
            | Opcode::SetOpS(..)
            | Opcode::SetRangeM(..)
            | Opcode::SetS(..)
            | Opcode::Shl
            | Opcode::Shr
            | Opcode::StaticAnalysisError
            | Opcode::String(..)
            | Opcode::Sub
            | Opcode::Switch(..)
            | Opcode::This
            | Opcode::Throw
            | Opcode::ThrowAsTypeStructException(_)
            | Opcode::ThrowNonExhaustiveSwitch
            | Opcode::True
            | Opcode::UnsetG
            | Opcode::UnsetM(..)
            | Opcode::Vec(..)
            | Opcode::VerifyOutType(..)
            | Opcode::VerifyParamType(..)
            | Opcode::VerifyParamTypeTS(..)
            | Opcode::VerifyRetNonNullC
            | Opcode::VerifyRetTypeC
            | Opcode::VerifyRetTypeTS
            | Opcode::VerifyTypeTS
            | Opcode::WHResult
            | Opcode::Yield
            | Opcode::YieldK => LocalInfo::None,
        }
    }

    pub(crate) fn for_node(instr: &NodeInstr) -> LocalInfo {
        match instr {
            NodeInstr::Opcode(opcode) => LocalInfo::for_opcode(opcode),
            NodeInstr::MemberOp(_) => LocalInfo::None,
        }
    }
}

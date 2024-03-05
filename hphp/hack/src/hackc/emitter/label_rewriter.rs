// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use env::LabelGen;
use hash::HashMap;
use hash::HashSet;
use hhbc::Instruct;
use hhbc::Label;
use hhbc::Opcode;
use hhbc::Param;
use hhbc::Pseudo;
use instruction_sequence::InstrSeq;
use oxidized::ast;

/// Create a mapping Label instructions to their position in the InstrSeq without
/// the labels. In other words, all instructions get numbered except labels.
fn create_label_to_offset_map(instrseq: &InstrSeq) -> HashMap<Label, u32> {
    let mut index = 0;
    instrseq
        .iter()
        .filter_map(|instr| match instr {
            Instruct::Pseudo(Pseudo::Label(label)) => Some((*label, index)),
            _ => {
                index += 1;
                None
            }
        })
        .collect()
}

fn create_label_ref_map(
    label_to_offset: &HashMap<Label, u32>,
    params: &[(Param, Option<(Label, ast::Expr)>)],
    body: &InstrSeq,
) -> (HashSet<Label>, HashMap<u32, Label>) {
    let mut label_gen = LabelGen::new();
    let mut used = HashSet::default();
    let mut offset_to_label = HashMap::default();

    let mut process_ref = |target: &Label| {
        let offset = label_to_offset[target];
        offset_to_label.entry(offset).or_insert_with(|| {
            used.insert(*target);
            label_gen.next_regular()
        });
    };

    // Process the function body.
    for instr in body.iter() {
        for target in instr.targets().iter() {
            process_ref(target);
        }
    }

    // Process params
    for (_param, dv) in params {
        if let Some((target, _)) = dv {
            process_ref(target);
        }
    }
    (used, offset_to_label)
}

fn rewrite_params_and_body(
    label_to_offset: &HashMap<Label, u32>,
    used: &HashSet<Label>,
    offset_to_label: &HashMap<u32, Label>,
    params: &mut [(Param, Option<(Label, ast::Expr)>)],
    body: &mut InstrSeq,
) {
    let relabel = |id: Label| {
        if id == Label::INVALID {
            Label::INVALID
        } else {
            offset_to_label[&label_to_offset[&id]]
        }
    };
    for (_, dv) in params.iter_mut() {
        if let Some((l, _)) = dv {
            *l = relabel(*l);
        }
    }
    body.retain_mut(|instr| {
        if let Instruct::Pseudo(Pseudo::Label(l)) = instr {
            if used.contains(l) {
                *l = relabel(*l);
                true
            } else {
                false
            }
        } else {
            rewrite_labels(instr, relabel);
            true
        }
    });
}

pub fn relabel_function(params: &mut [(Param, Option<(Label, ast::Expr)>)], body: &mut InstrSeq) {
    let label_to_offset = create_label_to_offset_map(body);
    let (used, offset_to_label) = create_label_ref_map(&label_to_offset, params, body);
    rewrite_params_and_body(&label_to_offset, &used, &offset_to_label, params, body)
}

pub fn rewrite_with_fresh_regular_labels(emitter: &mut Emitter<'_>, block: &mut InstrSeq) {
    let mut old_to_new = HashMap::default();
    for instr in block.iter() {
        if let Instruct::Pseudo(Pseudo::Label(label)) = instr {
            old_to_new.insert(*label, emitter.label_gen_mut().next_regular());
        }
    }

    if !old_to_new.is_empty() {
        let relabel = |target: Label| old_to_new.get(&target).copied().unwrap_or(target);
        for instr in block.iter_mut() {
            rewrite_labels(instr, relabel);
        }
    }
}

/// Apply the given function to every Label in the Instruct.
///
/// If this turns out to be needed elsewhere it should probably be moved into the
/// Targets trait.
fn rewrite_labels<F>(instr: &mut Instruct, f: F)
where
    F: Fn(Label) -> Label,
{
    match instr {
        Instruct::Pseudo(Pseudo::Label(label))
        | Instruct::Opcode(
            Opcode::Enter(label)
            | Opcode::IterInit(_, label)
            | Opcode::IterNext(_, label)
            | Opcode::Jmp(label)
            | Opcode::JmpNZ(label)
            | Opcode::JmpZ(label)
            | Opcode::LIterInit(_, _, label)
            | Opcode::LIterNext(_, _, label)
            | Opcode::MemoGet(label, _),
        ) => {
            *label = f(*label);
        }
        Instruct::Opcode(
            Opcode::FCallClsMethod(fca, _, _)
            | Opcode::FCallClsMethodD(fca, _, _)
            | Opcode::FCallClsMethodM(fca, _, _, _)
            | Opcode::FCallClsMethodS(fca, _, _)
            | Opcode::FCallClsMethodSD(fca, _, _, _)
            | Opcode::FCallCtor(fca, _)
            | Opcode::FCallFunc(fca)
            | Opcode::FCallFuncD(fca, _)
            | Opcode::FCallObjMethod(fca, _, _)
            | Opcode::FCallObjMethodD(fca, _, _, _),
        ) => {
            fca.async_eager_target = f(fca.async_eager_target);
        }
        Instruct::Opcode(Opcode::MemoGetEager([label1, label2], _, _)) => {
            *label1 = f(*label1);
            *label2 = f(*label2);
        }
        Instruct::Opcode(Opcode::Switch(_, _, labels)) => {
            for label in labels.as_mut_slice() {
                *label = f(*label);
            }
        }
        Instruct::Opcode(Opcode::SSwitch(_, targets)) => {
            for label in targets.as_mut_slice() {
                *label = f(*label);
            }
        }
        Instruct::Pseudo(
            Pseudo::Break
            | Pseudo::Continue
            | Pseudo::SrcLoc(..)
            | Pseudo::TryCatchBegin
            | Pseudo::TryCatchEnd
            | Pseudo::TryCatchMiddle,
        )
        | Instruct::Opcode(
            Opcode::AKExists
            | Opcode::AddElemC
            | Opcode::AddNewElemC
            | Opcode::Add
            | Opcode::ArrayIdx
            | Opcode::ArrayMarkLegacy
            | Opcode::ArrayUnmarkLegacy
            | Opcode::AssertRATL(..)
            | Opcode::AssertRATStk(..)
            | Opcode::AwaitAll(..)
            | Opcode::Await
            | Opcode::BareThis(..)
            | Opcode::BaseC(..)
            | Opcode::BaseGC(..)
            | Opcode::BaseGL(..)
            | Opcode::BaseH
            | Opcode::BaseL(..)
            | Opcode::BaseSC(..)
            | Opcode::BitAnd
            | Opcode::BitNot
            | Opcode::BitOr
            | Opcode::BitXor
            | Opcode::BreakTraceHint
            | Opcode::CGetCUNop
            | Opcode::CGetG
            | Opcode::CGetL(..)
            | Opcode::CGetL2(..)
            | Opcode::CGetQuietL(..)
            | Opcode::CGetS(..)
            | Opcode::CUGetL(..)
            | Opcode::CastBool
            | Opcode::CastDict
            | Opcode::CastDouble
            | Opcode::CastInt
            | Opcode::CastKeyset
            | Opcode::CastString
            | Opcode::CastVec
            | Opcode::ChainFaults
            | Opcode::CheckClsReifiedGenericMismatch
            | Opcode::CheckClsRGSoft
            | Opcode::CheckProp(..)
            | Opcode::CheckThis
            | Opcode::ClassGetC(..)
            | Opcode::ClassGetTS
            | Opcode::ClassHasReifiedGenerics
            | Opcode::ClassName
            | Opcode::Clone
            | Opcode::ClsCns(..)
            | Opcode::ClsCnsD(..)
            | Opcode::ClsCnsL(..)
            | Opcode::Cmp
            | Opcode::CnsE(..)
            | Opcode::ColFromArray(..)
            | Opcode::CombineAndResolveTypeStruct(..)
            | Opcode::ConcatN(..)
            | Opcode::Concat
            | Opcode::ContCheck(..)
            | Opcode::ContCurrent
            | Opcode::ContEnter
            | Opcode::ContGetReturn
            | Opcode::ContKey
            | Opcode::ContRaise
            | Opcode::ContValid
            | Opcode::CreateCl(..)
            | Opcode::CreateCont
            | Opcode::CreateSpecialImplicitContext
            | Opcode::DblAsBits
            | Opcode::Dict(..)
            | Opcode::Dim(..)
            | Opcode::Dir
            | Opcode::Div
            | Opcode::Double(..)
            | Opcode::Dup
            | Opcode::EnumClassLabel(..)
            | Opcode::EnumClassLabelName
            | Opcode::Eq
            | Opcode::Eval
            | Opcode::Exit
            | Opcode::False
            | Opcode::Fatal(..)
            | Opcode::File
            | Opcode::FuncCred
            | Opcode::GetClsRGProp
            | Opcode::GetMemoKeyL(..)
            | Opcode::Gte
            | Opcode::Gt
            | Opcode::HasReifiedParent
            | Opcode::Idx
            | Opcode::IncDecG(..)
            | Opcode::IncDecL(..)
            | Opcode::IncDecM(..)
            | Opcode::IncDecS(..)
            | Opcode::InclOnce
            | Opcode::Incl
            | Opcode::InitProp(..)
            | Opcode::InstanceOfD(..)
            | Opcode::InstanceOf
            | Opcode::Int(..)
            | Opcode::IsLateBoundCls
            | Opcode::IsTypeC(..)
            | Opcode::IsTypeL(..)
            | Opcode::IsTypeStructC(..)
            | Opcode::IsUnsetL(..)
            | Opcode::IssetG
            | Opcode::IssetL(..)
            | Opcode::IssetS
            | Opcode::IterFree(..)
            | Opcode::Keyset(..)
            | Opcode::LIterFree(..)
            | Opcode::LateBoundCls
            | Opcode::LazyClass(..)
            | Opcode::LazyClassFromClass
            | Opcode::LockObj
            | Opcode::Lte
            | Opcode::Lt
            | Opcode::MemoSet(..)
            | Opcode::MemoSetEager(..)
            | Opcode::Method
            | Opcode::Mod
            | Opcode::Mul
            | Opcode::NSame
            | Opcode::NativeImpl
            | Opcode::Neq
            | Opcode::NewCol(..)
            | Opcode::NewDictArray(..)
            | Opcode::NewKeysetArray(..)
            | Opcode::NewObjD(..)
            | Opcode::NewObjS(..)
            | Opcode::NewObj
            | Opcode::NewPair
            | Opcode::NewStructDict(..)
            | Opcode::NewVec(..)
            | Opcode::Nop
            | Opcode::Not
            | Opcode::NullUninit
            | Opcode::Null
            | Opcode::OODeclExists(..)
            | Opcode::ParentCls
            | Opcode::PopC
            | Opcode::PopL(..)
            | Opcode::PopU2
            | Opcode::PopU
            | Opcode::Pow
            | Opcode::Print
            | Opcode::PushL(..)
            | Opcode::QueryM(..)
            | Opcode::RaiseClassStringConversionNotice
            | Opcode::RecordReifiedGeneric
            | Opcode::ReqDoc
            | Opcode::ReqOnce
            | Opcode::Req
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
            | Opcode::RetCSuspended
            | Opcode::RetC
            | Opcode::RetM(..)
            | Opcode::Same
            | Opcode::Select
            | Opcode::SelfCls
            | Opcode::SetG
            | Opcode::SetImplicitContextByValue
            | Opcode::SetL(..)
            | Opcode::SetM(..)
            | Opcode::SetOpG(..)
            | Opcode::SetOpL(..)
            | Opcode::SetOpM(..)
            | Opcode::SetOpS(..)
            | Opcode::SetRangeM(..)
            | Opcode::SetS(..)
            | Opcode::Shl
            | Opcode::Shr
            | Opcode::Silence(..)
            | Opcode::String(..)
            | Opcode::Sub
            | Opcode::This
            | Opcode::ThrowAsTypeStructException(..)
            | Opcode::ThrowNonExhaustiveSwitch
            | Opcode::Throw
            | Opcode::True
            | Opcode::UGetCUNop
            | Opcode::UnsetG
            | Opcode::UnsetL(..)
            | Opcode::UnsetM(..)
            | Opcode::Vec(..)
            | Opcode::VerifyImplicitContextState
            | Opcode::VerifyOutType(..)
            | Opcode::VerifyParamType(..)
            | Opcode::VerifyParamTypeTS(..)
            | Opcode::VerifyRetNonNullC
            | Opcode::VerifyRetTypeC
            | Opcode::VerifyRetTypeTS
            | Opcode::WHResult
            | Opcode::YieldK
            | Opcode::Yield,
        ) => {
            debug_assert!(
                instr.targets().is_empty(),
                "bad instr {instr:?} shouldn't have targets"
            );
        }
    }
}

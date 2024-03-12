use anyhow::bail;
use anyhow::Result;
use ffi::Vector;
use hhbc::Opcode;
use itertools::Itertools;
use log::trace;

use crate::code_path::CodePath;
use crate::helpers::*;
use crate::node::Input;
use crate::node::Node;
use crate::node::NodeInstr;

pub(crate) struct Sequence<'a> {
    pub(crate) debug_name: String,
    pub(crate) instrs: Vec<Node<'a>>,
    // If the sequence ends on a loopback (so we terminate it early to avoid
    // infinite loops) then this tells us where it looped back to.
    pub(crate) loopback: Option<usize>,
}

impl<'a> Sequence<'a> {
    pub(crate) fn compare(path: &CodePath<'_>, a: Self, b: Self) -> Result<()> {
        trace!("--- Compare {} and {}", a.debug_name, b.debug_name);

        let seq_a = collect_sequence(&a, b.instrs.len());
        let seq_b = collect_sequence(&b, a.instrs.len());

        let mut it_a = seq_a.iter().copied();
        let mut it_b = seq_b.iter().copied();

        let mut idx = 0;
        loop {
            let instr_a = it_a.next();
            let instr_b = it_b.next();

            let (instr_a, instr_b) = match (instr_a, instr_b) {
                (None, None) => break,
                (Some(_), None) | (None, Some(_)) => return bail_early_end(path, instr_a, instr_b),
                (Some(a), Some(b)) => (a, b),
            };

            trace!("  COMPARE\n    {:?}\n    {:?}", instr_a, instr_b);

            compare_instrs(&path.index(idx), &instr_a.instr, &instr_b.instr)?;

            let cow_inputs = is_cow_instr(&instr_a.instr);

            sem_diff_slice(
                &path.index(idx).qualified("inputs"),
                &instr_a.inputs,
                &instr_b.inputs,
                |p, a, b| sem_diff_input(p, a, b, cow_inputs),
            )?;

            sem_diff_eq(
                &path.index(idx).qualified("src_loc"),
                &instr_a.src_loc,
                &instr_b.src_loc,
            )?;
            idx += 1;
        }

        trace!("  - compare done");
        Ok(())
    }
}

fn compare_instrs(path: &CodePath<'_>, a: &NodeInstr, b: &NodeInstr) -> Result<()> {
    // Note: If the thing that's different is a Label that's
    // actually okay because we track labels independently.

    if std::mem::discriminant(a) != std::mem::discriminant(b) {
        bail!("Mismatch in {}:\n{:?}\n{:?}", path, a, b);
    }

    use NodeInstr as I;
    use Opcode as O;

    match (a, b) {
        (I::Opcode(O::Enter(_)), I::Opcode(O::Enter(_)))
        | (I::Opcode(O::Jmp(_)), I::Opcode(O::Jmp(_)))
        | (I::Opcode(O::JmpNZ(_)), I::Opcode(O::JmpNZ(_)))
        | (I::Opcode(O::JmpZ(_)), I::Opcode(O::JmpZ(_))) => Ok(()),

        (I::Opcode(O::IterInit(a0, _)), I::Opcode(O::IterInit(a1, _))) => sem_diff_eq(path, a0, a1),
        (I::Opcode(O::IterNext(a0, _)), I::Opcode(O::IterNext(a1, _))) => sem_diff_eq(path, a0, a1),
        (I::Opcode(O::LIterInit(a0, b0, _)), I::Opcode(O::LIterInit(a1, b1, _))) => {
            sem_diff_eq(path, &(a0, b0), &(a1, b1))
        }
        (I::Opcode(O::LIterNext(a0, b0, _)), I::Opcode(O::LIterNext(a1, b1, _))) => {
            sem_diff_eq(path, &(a0, b0), &(a1, b1))
        }
        (I::Opcode(O::MemoGet(_, a0)), I::Opcode(O::MemoGet(_, a1))) => sem_diff_eq(path, a0, a1),
        (I::Opcode(O::MemoGetEager(_, _, a0)), I::Opcode(O::MemoGetEager(_, _, a1))) => {
            sem_diff_eq(path, a0, a1)
        }
        (I::Opcode(O::Switch(a0, b0, _)), I::Opcode(O::Switch(a1, b1, _))) => {
            sem_diff_eq(path, &(a0, b0), &(a1, b1))
        }

        (
            I::Opcode(O::FCallClsMethod(fca0, a0, b0)),
            I::Opcode(O::FCallClsMethod(fca1, a1, b1)),
        ) => {
            sem_diff_fca(path, fca0, fca1)?;
            sem_diff_eq(path, &(a0, b0), &(a1, b1))
        }
        (
            I::Opcode(O::FCallClsMethodD(fca0, a0, b0)),
            I::Opcode(O::FCallClsMethodD(fca1, a1, b1)),
        ) => {
            sem_diff_fca(path, fca0, fca1)?;
            sem_diff_eq(path, &(a0, b0), &(a1, b1))
        }
        (
            I::Opcode(O::FCallClsMethodS(fca0, a0, b0)),
            I::Opcode(O::FCallClsMethodS(fca1, a1, b1)),
        ) => {
            sem_diff_fca(path, fca0, fca1)?;
            sem_diff_eq(path, &(a0, b0), &(a1, b1))
        }
        (
            I::Opcode(O::FCallClsMethodSD(fca0, a0, b0, c0)),
            I::Opcode(O::FCallClsMethodSD(fca1, a1, b1, c1)),
        ) => {
            sem_diff_fca(path, fca0, fca1)?;
            sem_diff_eq(path, &(a0, b0, c0), &(a1, b1, c1))
        }
        (I::Opcode(O::FCallCtor(fca0, a0)), I::Opcode(O::FCallCtor(fca1, a1))) => {
            sem_diff_fca(path, fca0, fca1)?;
            sem_diff_eq(path, a0, a1)
        }
        (I::Opcode(O::FCallFunc(fca0)), I::Opcode(O::FCallFunc(fca1))) => {
            sem_diff_fca(path, fca0, fca1)
        }
        (I::Opcode(O::FCallFuncD(fca0, a0)), I::Opcode(O::FCallFuncD(fca1, a1))) => {
            sem_diff_fca(path, fca0, fca1)?;
            sem_diff_eq(path, a0, a1)
        }
        (
            I::Opcode(O::FCallObjMethod(fca0, a0, b0)),
            I::Opcode(O::FCallObjMethod(fca1, a1, b1)),
        ) => {
            sem_diff_fca(path, fca0, fca1)?;
            sem_diff_eq(path, &(a0, b0), &(a1, b1))
        }
        (
            I::Opcode(O::FCallObjMethodD(fca0, a0, b0, c0)),
            I::Opcode(O::FCallObjMethodD(fca1, a1, b1, c1)),
        ) => {
            sem_diff_fca(path, fca0, fca1)?;
            sem_diff_eq(path, &(a0, b0, c0), &(a1, b1, c1))
        }

        (I::Opcode(O::SSwitch(a0, _)), I::Opcode(O::SSwitch(a1, _))) => sem_diff_eq(path, a0, a1),

        (a, b) => {
            match a {
                I::Opcode(a) => {
                    use hhbc::Targets;
                    debug_assert!(
                        a.targets().iter().all(|target| !target.is_valid()),
                        "This instr has a target that should have been ignored"
                    );
                }
                _ => {}
            }
            sem_diff_eq(path, a, b)
        }
    }
}

fn sem_diff_input(
    path: &CodePath<'_>,
    input0: &Input<'_>,
    input1: &Input<'_>,
    check_cow: bool,
) -> Result<()> {
    if check_cow {
        match (input0, input1) {
            (Input::Read(a), Input::Read(b))
            | (Input::Unowned(a), Input::Unowned(b))
            | (Input::Owned(a), Input::Owned(b) | Input::Unowned(b))
            | (Input::Shared(a), Input::Shared(b) | Input::Owned(b) | Input::Unowned(b)) => {
                // We allow 'b' to be 'less owned' than 'a'.
                sem_diff_eq(path, a, b)
            }
            (a, b) => sem_diff_eq(path, a, b),
        }
    } else {
        match (input0, input1) {
            (
                Input::Read(a) | Input::Unowned(a) | Input::Owned(a) | Input::Shared(a),
                Input::Read(b) | Input::Unowned(b) | Input::Owned(b) | Input::Shared(b),
            ) => sem_diff_eq(path, a, b),
            (a, b) => sem_diff_eq(path, a, b),
        }
    }
}

/// Returns true if this is an instruction that could cause a COW.
fn is_cow_instr(instr: &NodeInstr) -> bool {
    match instr {
        // Constants
        NodeInstr::Opcode(
            Opcode::Dict(..)
            | Opcode::Dir
            | Opcode::Double(..)
            | Opcode::EnumClassLabel(..)
            | Opcode::False
            | Opcode::File
            | Opcode::FuncCred
            | Opcode::Int(..)
            | Opcode::Keyset(..)
            | Opcode::LazyClass(..)
            | Opcode::LazyClassFromClass
            | Opcode::NewCol(..)
            | Opcode::Null
            | Opcode::NullUninit
            | Opcode::String(..)
            | Opcode::This
            | Opcode::True
            | Opcode::Vec(..),
        ) => false,

        // Stack/Local manipulation
        NodeInstr::Opcode(
            Opcode::CGetL(..)
            | Opcode::CGetL2(..)
            | Opcode::CnsE(..)
            | Opcode::Dup
            | Opcode::Lt
            | Opcode::Mod
            | Opcode::PopC
            | Opcode::PopL(..)
            | Opcode::PushL(..)
            | Opcode::Same
            | Opcode::SetL(..)
            | Opcode::UnsetL(..)
            | Opcode::CGetCUNop
            | Opcode::CGetG
            | Opcode::CGetQuietL(..)
            | Opcode::CGetS(..)
            | Opcode::CUGetL(..),
        ) => false,

        // Is operations
        NodeInstr::Opcode(
            Opcode::InstanceOf
            | Opcode::InstanceOfD(..)
            | Opcode::IsLateBoundCls
            | Opcode::IsTypeC(..)
            | Opcode::IsTypeL(..)
            | Opcode::IsTypeStructC(..)
            | Opcode::IsUnsetL(..)
            | Opcode::IssetG
            | Opcode::IssetL(..)
            | Opcode::IssetS,
        ) => false,

        // Control flow
        NodeInstr::Opcode(
            Opcode::Enter(_)
            | Opcode::Jmp(_)
            | Opcode::Nop
            | Opcode::JmpNZ(..)
            | Opcode::JmpZ(..)
            | Opcode::RetC
            | Opcode::RetCSuspended
            | Opcode::RetM(..)
            | Opcode::SSwitch { .. }
            | Opcode::Switch(..)
            | Opcode::Throw
            | Opcode::ThrowAsTypeStructException(_)
            | Opcode::ThrowNonExhaustiveSwitch,
        ) => false,

        // Other Opcodes
        NodeInstr::Opcode(Opcode::Concat | Opcode::ConcatN(..)) => false,

        // Operators
        NodeInstr::Opcode(
            Opcode::Add
            | Opcode::BitAnd
            | Opcode::BitNot
            | Opcode::BitOr
            | Opcode::BitXor
            | Opcode::Cmp
            | Opcode::Div
            | Opcode::Eq
            | Opcode::Gt
            | Opcode::Gte
            | Opcode::Idx
            | Opcode::Lte
            | Opcode::Mul
            | Opcode::NSame
            | Opcode::Neq
            | Opcode::Not
            | Opcode::Pow
            | Opcode::Print
            | Opcode::Select
            | Opcode::Shl
            | Opcode::Shr
            | Opcode::Sub
            | Opcode::AKExists
            | Opcode::ArrayIdx
            | Opcode::AssertRATL(..)
            | Opcode::AssertRATStk(..)
            | Opcode::Await
            | Opcode::AwaitAll(..)
            | Opcode::BareThis(..)
            | Opcode::BreakTraceHint
            | Opcode::CheckProp(..)
            | Opcode::CheckClsReifiedGenericMismatch
            | Opcode::CheckClsRGSoft
            | Opcode::CheckThis
            | Opcode::ClassGetC(..)
            | Opcode::ClassGetTS
            | Opcode::ClassHasReifiedGenerics
            | Opcode::ClassName
            | Opcode::ClsCns(..)
            | Opcode::ClsCnsD(..)
            | Opcode::ClsCnsL(..)
            | Opcode::ContCheck(..)
            | Opcode::ContCurrent
            | Opcode::ContEnter
            | Opcode::ContGetReturn
            | Opcode::ContKey
            | Opcode::ContRaise
            | Opcode::ContValid
            | Opcode::CreateCont
            | Opcode::CreateSpecialImplicitContext
            | Opcode::DblAsBits
            | Opcode::EnumClassLabelName
            | Opcode::Exit
            | Opcode::GetClsRGProp
            | Opcode::GetMemoKeyL(..)
            | Opcode::MemoGet(..)
            | Opcode::MemoGetEager(..)
            | Opcode::ParentCls
            | Opcode::PopU
            | Opcode::PopU2,
        ) => false,

        // Casting
        NodeInstr::Opcode(
            Opcode::CastBool
            | Opcode::CastDict
            | Opcode::CastDouble
            | Opcode::CastInt
            | Opcode::CastKeyset
            | Opcode::CastString
            | Opcode::CastVec,
        ) => false,

        // Verify
        NodeInstr::Opcode(
            Opcode::VerifyImplicitContextState
            | Opcode::VerifyOutType(..)
            | Opcode::VerifyParamType(..)
            | Opcode::VerifyParamTypeTS(..)
            | Opcode::VerifyRetNonNullC
            | Opcode::VerifyRetTypeC
            | Opcode::VerifyRetTypeTS,
        ) => false,

        NodeInstr::Opcode(
            Opcode::AddElemC
            | Opcode::AddNewElemC
            | Opcode::ArrayMarkLegacy
            | Opcode::ArrayUnmarkLegacy
            | Opcode::BaseC(..)
            | Opcode::BaseGC(..)
            | Opcode::BaseGL(..)
            | Opcode::BaseH
            | Opcode::BaseL(..)
            | Opcode::BaseSC(..)
            | Opcode::ChainFaults
            | Opcode::Clone
            | Opcode::ColFromArray(..)
            | Opcode::CombineAndResolveTypeStruct(..)
            | Opcode::CreateCl(..)
            | Opcode::Dim(..)
            | Opcode::Eval
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
            | Opcode::Fatal(..)
            | Opcode::HasReifiedParent
            | Opcode::IncDecG(..)
            | Opcode::IncDecL(..)
            | Opcode::IncDecM(..)
            | Opcode::IncDecS(..)
            | Opcode::Incl
            | Opcode::InclOnce
            | Opcode::InitProp(..)
            | Opcode::IterFree(..)
            | Opcode::IterInit(..)
            | Opcode::IterNext(..)
            | Opcode::LIterFree(..)
            | Opcode::LIterInit(..)
            | Opcode::LIterNext(..)
            | Opcode::LateBoundCls
            | Opcode::LockObj
            | Opcode::MemoSet(..)
            | Opcode::MemoSetEager(..)
            | Opcode::Method
            | Opcode::NativeImpl
            | Opcode::NewDictArray(..)
            | Opcode::NewKeysetArray(..)
            | Opcode::NewObj
            | Opcode::NewObjD(..)
            | Opcode::NewObjS(..)
            | Opcode::NewPair
            | Opcode::NewStructDict(..)
            | Opcode::NewVec(..)
            | Opcode::OODeclExists(..)
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
            | Opcode::SelfCls
            | Opcode::SetG
            | Opcode::SetImplicitContextByValue
            | Opcode::SetM(..)
            | Opcode::SetOpG(..)
            | Opcode::SetOpL(..)
            | Opcode::SetOpM(..)
            | Opcode::SetOpS(..)
            | Opcode::SetRangeM(..)
            | Opcode::SetS(..)
            | Opcode::Silence(..)
            | Opcode::UGetCUNop
            | Opcode::UnsetG
            | Opcode::UnsetM(..)
            | Opcode::WHResult
            | Opcode::Yield
            | Opcode::YieldK,
        ) => true,

        NodeInstr::MemberOp(_) => true,
    }
}

fn sem_diff_fca(path: &CodePath<'_>, fca0: &hhbc::FCallArgs, fca1: &hhbc::FCallArgs) -> Result<()> {
    let hhbc::FCallArgs {
        flags: flags0,
        async_eager_target: _,
        num_args: num_args0,
        num_rets: num_rets0,
        inouts: inouts0,
        readonly: readonly0,
        context: context0,
    } = fca0;
    let hhbc::FCallArgs {
        flags: flags1,
        async_eager_target: _,
        num_args: num_args1,
        num_rets: num_rets1,
        inouts: inouts1,
        readonly: readonly1,
        context: context1,
    } = fca1;

    fn cmp_slice_where_empty_is_all_false(
        path: &CodePath<'_>,
        a: &Vector<bool>,
        b: &Vector<bool>,
    ) -> Result<()> {
        match (a.is_empty(), b.is_empty()) {
            (true, true) => {}
            (true, false) => {
                if b.iter().any(|x| *x) {
                    bail!("Mismatch in {}:\n{:?}\n{:?}", path, a, b);
                }
            }
            (false, true) => {
                if a.iter().any(|x| *x) {
                    bail!("Mismatch in {}:\n{:?}\n{:?}", path, a, b);
                }
            }
            (false, false) => sem_diff_eq(path, a, b)?,
        }
        Ok(())
    }

    cmp_slice_where_empty_is_all_false(&path.qualified("inouts"), inouts0, inouts1)?;
    cmp_slice_where_empty_is_all_false(&path.qualified("readonly"), readonly0, readonly1)?;

    sem_diff_eq(
        path,
        &(flags0, num_args0, num_rets0, context0),
        &(flags1, num_args1, num_rets1, context1),
    )
}

fn bail_early_end(
    path: &CodePath<'_>,
    instr_a: Option<&Node<'_>>,
    instr_b: Option<&Node<'_>>,
) -> Result<()> {
    bail!(
        "Mismatch in {}:
{:?}
{:?}

One side ended before the other. This can happen due to a bytecode mismatch or
due to a try/catch mismatch",
        path,
        instr_a,
        instr_b
    );
}

fn collect_sequence<'a>(seq: &'a Sequence<'a>, min_len: usize) -> Vec<&'a Node<'a>> {
    if min_len <= seq.instrs.len() || seq.loopback.is_none() {
        return seq.instrs.iter().collect_vec();
    }

    let loopback = seq.loopback.unwrap();
    seq.instrs
        .iter()
        .chain(seq.instrs[loopback..].iter().cycle())
        .take(min_len)
        .collect_vec()
}

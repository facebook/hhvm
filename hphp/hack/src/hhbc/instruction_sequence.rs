// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use env::iterator::Iter;
use hhbc_ast_rust::*;
use label_rust::Label;
use local_rust as local;
use runtime::TypedValue;

use std::collections::HashMap;
use std::convert::TryInto;

/// The various from_X functions below take some kind of AST (expression,
/// statement, etc.) and produce what is logically a sequence of instructions.
/// This could simply be represented by a list, but then we would need to
/// use an accumulator to avoid the quadratic complexity associated with
/// repeated appending to a list. Instead, we simply build a tree of
/// instructions which can easily be flattened at the end.
#[derive(Clone, Debug)]
pub enum Instr {
    Empty,
    One(Box<Instruct>),
    List(Vec<Instruct>),
    Concat(Vec<Instr>),
}

impl Instr {
    fn gather(instrs: Vec<Self>) -> Self {
        let nonempty_instrs = instrs
            .into_iter()
            .filter(|x| match x {
                Self::Empty => false,
                _ => true,
            })
            .collect::<Vec<_>>();
        match &nonempty_instrs[..] {
            [] => Self::Empty,
            [x] => x.clone(),
            xs => Self::Concat(xs.to_vec()),
        }
    }

    fn make_empty() -> Self {
        Self::Empty
    }

    fn make_instr(instruction: Instruct) -> Self {
        Self::One(Box::new(instruction))
    }

    fn make_instrs(instructions: Vec<Instruct>) -> Self {
        Self::List(instructions)
    }

    fn make_instr_lit_const(l: InstructLitConst) -> Self {
        Self::make_instr(Instruct::ILitConst(l))
    }

    fn make_instr_lit_empty_varray() -> Self {
        Self::make_instr_lit_const(InstructLitConst::TypedValue(TypedValue::VArray(vec![])))
    }

    fn make_instr_iterinit(iter_id: Iter, label: Label, value: local::Id) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterInit(
            iter_id, label, value,
        )))
    }

    fn make_instr_iterinitk(id: Iter, label: Label, key: local::Id, value: local::Id) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterInitK(
            id, label, key, value,
        )))
    }

    fn make_instr_iternext(id: Iter, label: Label, value: local::Id) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterNext(
            id, label, value,
        )))
    }

    fn make_instr_iternextk(id: Iter, label: Label, key: local::Id, value: local::Id) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterNextK(
            id, label, key, value,
        )))
    }

    fn make_instr_iterfree(id: Iter) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterFree(id)))
    }

    fn make_instr_whresult() -> Self {
        Self::make_instr(Instruct::IAsync(AsyncFunctions::WHResult))
    }

    fn make_instr_jmp(label: Label) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::Jmp(label)))
    }

    fn make_instr_jmpz(label: Label) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::JmpZ(label)))
    }

    fn make_instr_jmpnz(label: Label) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::JmpNZ(label)))
    }

    fn make_instr_jmpns(label: Label) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::JmpNS(label)))
    }

    fn make_instr_continue(level: isize) -> Self {
        Self::make_instr(Instruct::ISpecialFlow(InstructSpecialFlow::Continue(level)))
    }

    fn make_instr_break(level: isize) -> Self {
        Self::make_instr(Instruct::ISpecialFlow(InstructSpecialFlow::Break(level)))
    }

    fn make_instr_goto(label: String) -> Self {
        Self::make_instr(Instruct::ISpecialFlow(InstructSpecialFlow::Goto(label)))
    }

    fn make_instr_iter_break(label: Label, itrs: Vec<(IterKind, Iter)>) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterBreak(
            label, itrs,
        )))
    }

    fn make_instr_false() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::False))
    }

    fn make_instr_true() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::True))
    }

    fn make_instr_eq() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Eq))
    }

    fn make_instr_gt() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Gt))
    }

    fn make_instr_concat() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Concat))
    }

    fn make_instr_concatn(n: isize) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ConcatN(n)))
    }

    fn make_instr_print() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Print))
    }

    fn make_instr_cast_darray() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::CastDArray))
    }

    fn make_instr_cast_dict() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::CastDict))
    }

    fn make_instr_retc() -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::RetC))
    }

    fn make_instr_retc_suspended() -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::RetCSuspended))
    }

    fn make_instr_retm(p: NumParams) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::RetM(p)))
    }

    fn make_instr_null() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Null))
    }

    fn make_instr_nulluninit() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NullUninit))
    }

    fn make_instr_chain_faults() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::ChainFaults))
    }

    fn make_instr_dup() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::Dup))
    }

    fn make_instr_nop() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::Nop))
    }

    fn make_instr_instanceofd(s: ClassId) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::InstanceOfD(s)))
    }

    fn make_instr_instanceof() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::InstanceOf))
    }

    fn make_instr_islateboundcls() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::IsLateBoundCls))
    }

    fn make_instr_istypestructc(mode: TypestructResolveOp) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::IsTypeStructC(mode)))
    }

    fn make_instr_throwastypestructexception() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ThrowAsTypeStructException))
    }

    fn make_instr_combine_and_resolve_type_struct(i: isize) -> Self {
        Self::make_instr(Instruct::IOp(
            InstructOperator::CombineAndResolveTypeStruct(i),
        ))
    }

    fn make_instr_record_reified_generic() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::RecordReifiedGeneric))
    }

    fn make_instr_check_reified_generic_mismatch() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::CheckReifiedGenericMismatch))
    }

    fn make_instr_int(i: isize) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Int(
            i.try_into().unwrap(),
        )))
    }

    fn make_instr_int64(i: i64) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Int(i)))
    }

    fn make_instr_int_of_string(litstr: &str) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Int(
            litstr.parse::<i64>().unwrap(),
        )))
    }

    fn make_instr_double(litstr: &str) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Double(
            litstr.to_string(),
        )))
    }

    fn make_instr_string(litstr: &str) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::String(
            litstr.to_string(),
        )))
    }

    fn make_instr_this() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::This))
    }

    fn make_instr_istypec(op: IstypeOp) -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IsTypeC(op)))
    }

    fn make_instr_istypel(id: local::Id, op: IstypeOp) -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IsTypeL(id, op)))
    }

    fn make_instr_not() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Not))
    }

    fn make_instr_sets() -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::SetS))
    }

    fn make_instr_setl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::SetL(local)))
    }

    fn make_instr_unsetl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::UnsetL(local)))
    }

    fn make_instr_issetl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IssetL(local)))
    }

    fn make_instr_issetg() -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IssetG))
    }

    fn make_instr_issets() -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IssetS))
    }

    fn make_instr_emptys() -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::EmptyS))
    }

    fn make_instr_emptyg() -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::EmptyG))
    }

    fn make_instr_emptyl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::EmptyL(local)))
    }

    fn make_instr_cgets() -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetS))
    }

    fn make_instr_cgetg() -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetG))
    }

    fn make_instr_cgetl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetL(local)))
    }

    fn make_instr_cugetl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CUGetL(local)))
    }

    fn make_instr_vgetl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::VGetL(local)))
    }

    fn make_instr_cgetl2(local: local::Id) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetL2(local)))
    }

    fn make_instr_cgetquietl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetQuietL(local)))
    }

    fn make_instr_classgetc() -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::ClassGetC))
    }

    fn make_instr_classgetts() -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::ClassGetTS))
    }

    fn make_instr_classname() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::ClassName))
    }

    fn make_instr_self() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Self_))
    }

    fn make_instr_lateboundcls() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::LateBoundCls))
    }

    fn make_instr_parent() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Parent))
    }

    fn make_instr_popu() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::PopU))
    }

    fn make_instr_popc() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::PopC))
    }

    fn make_instr_popl(l: local::Type) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::PopL(l)))
    }

    fn make_instr_pushl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::PushL(local)))
    }

    fn make_instr_throw() -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::Throw))
    }

    fn make_instr_new_vec_array(i: isize) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewVecArray(i)))
    }

    fn make_instr_add_elemc() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::AddElemC))
    }

    fn make_instr_add_new_elemc() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::AddNewElemC))
    }

    fn make_instr_switch(labels: Vec<Label>) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::Switch(
            Switchkind::Unbounded,
            0,
            labels,
        )))
    }

    fn make_instr_newobj() -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObj))
    }

    fn make_instr_newobjr() -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObjR))
    }

    fn make_instr_newobjd(id: ClassId) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObjD(id)))
    }

    fn make_instr_newobjrd(id: ClassId) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObjRD(id)))
    }

    fn make_instr_newobjs(scref: SpecialClsRef) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObjS(scref)))
    }

    fn make_instr_lockobj() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::LockObj))
    }

    fn make_instr_clone() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Clone))
    }

    fn make_instr_new_record(id: ClassId, keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewRecord(id, keys)))
    }

    fn make_instr_new_recordarray(id: ClassId, keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewRecordArray(
            id, keys,
        )))
    }

    fn make_instr_newstructarray(keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewStructArray(keys)))
    }

    fn make_instr_newstructdarray(keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewStructDArray(keys)))
    }

    fn make_instr_newstructdict(keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewStructDict(keys)))
    }

    fn make_instr_newcol(collection_type: CollectionType) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewCol(
            collection_type,
        )))
    }

    fn make_instr_colfromarray(collection_type: CollectionType) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::ColFromArray(
            collection_type,
        )))
    }

    fn make_instr_entrynop() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::EntryNop))
    }

    fn make_instr_typedvalue(xs: TypedValue) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::TypedValue(xs)))
    }

    fn make_instr_basel(local: local::Id, mode: MemberOpMode) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseL(local, mode)))
    }

    fn make_instr_basec(stack_index: StackIndex, mode: MemberOpMode) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseC(stack_index, mode)))
    }

    fn make_instr_basesc(y: StackIndex, z: StackIndex, mode: MemberOpMode) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseSC(y, z, mode)))
    }

    fn make_instr_baseh() -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseH))
    }

    fn make_instr_cgetcunop() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::CGetCUNop))
    }

    fn make_instr_ugetcunop() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::UGetCUNop))
    }

    fn make_instr_memoget(label: Label, range: Option<(local::Id, isize)>) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::MemoGet(label, range)))
    }

    fn make_instr_memoget_eager(
        label1: Label,
        label2: Label,
        range: Option<(local::Id, isize)>,
    ) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::MemoGetEager(
            label1, label2, range,
        )))
    }

    fn make_instr_memoset(range: Option<(local::Id, isize)>) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::MemoSet(range)))
    }

    fn make_instr_memoset_eager(range: Option<(local::Id, isize)>) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::MemoSetEager(range)))
    }

    fn make_instr_getmemokeyl(local: local::Id) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::GetMemoKeyL(local)))
    }

    fn make_instr_checkthis() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::CheckThis))
    }

    fn make_instr_verify_ret_type_c() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::VerifyRetTypeC))
    }

    fn make_instr_verify_ret_type_ts() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::VerifyRetTypeTS))
    }

    fn make_instr_verify_out_type(i: ParamId) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::VerifyOutType(i)))
    }

    fn make_instr_dim(op: MemberOpMode, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::Dim(op, key)))
    }

    fn make_instr_dim_warn_pt(key: PropId) -> Self {
        Self::make_instr_dim(MemberOpMode::Warn, MemberKey::PT(key))
    }

    fn make_instr_dim_define_pt(key: PropId) -> Self {
        Self::make_instr_dim(MemberOpMode::Define, MemberKey::PT(key))
    }

    fn make_instr_fcallclsmethod(
        is_log_as_dynamic_call: IsLogAsDynamicCallOp,
        fcall_args: FcallArgs,
        pl: ParamLocations,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallClsMethod(
            fcall_args,
            pl,
            is_log_as_dynamic_call,
        )))
    }

    fn make_instr_fcallclsmethodd(
        fcall_args: FcallArgs,
        method_name: MethodId,
        class_name: ClassId,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallClsMethodD(
            fcall_args,
            class_name,
            method_name,
        )))
    }

    fn make_instr_fcallclsmethods(fcall_args: FcallArgs, scref: SpecialClsRef) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallClsMethodS(
            fcall_args, scref,
        )))
    }

    fn make_instr_fcallclsmethodsd(
        fcall_args: FcallArgs,
        scref: SpecialClsRef,
        method_name: MethodId,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallClsMethodSD(
            fcall_args,
            scref,
            method_name,
        )))
    }

    fn make_instr_fcallctor(fcall_args: FcallArgs) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallCtor(fcall_args)))
    }

    fn make_instr_fcallfunc(fcall_args: FcallArgs, param_locs: ParamLocations) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallFunc(
            fcall_args, param_locs,
        )))
    }

    fn make_instr_fcallfuncd(fcall_args: FcallArgs, id: FunctionId) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallFuncD(fcall_args, id)))
    }

    fn make_instr_fcallobjmethod(
        fcall_args: FcallArgs,
        flavor: ObjNullFlavor,
        pl: ParamLocations,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallObjMethod(
            fcall_args, flavor, pl,
        )))
    }

    fn make_instr_fcallobjmethodd(
        fcall_args: FcallArgs,
        method: MethodId,
        flavor: ObjNullFlavor,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallObjMethodD(
            fcall_args, flavor, method,
        )))
    }

    fn make_instr_fcallobjmethodd_nullthrows(fcall_args: FcallArgs, method: MethodId) -> Self {
        Self::make_instr_fcallobjmethodd(fcall_args, method, ObjNullFlavor::NullThrows)
    }

    fn make_instr_querym(num_params: NumParams, op: QueryOp, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IFinal(InstructFinal::QueryM(num_params, op, key)))
    }

    fn make_instr_querym_cget_pt(num_params: NumParams, key: PropId) -> Self {
        Self::make_instr_querym(num_params, QueryOp::CGet, MemberKey::PT(key))
    }

    fn make_instr_setm(num_params: NumParams, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IFinal(InstructFinal::SetM(num_params, key)))
    }

    fn make_instr_setm_pt(num_params: NumParams, key: PropId) -> Self {
        Self::make_instr_setm(num_params, MemberKey::PT(key))
    }

    fn make_instr_resolve_func(func_id: FunctionId) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ResolveFunc(func_id)))
    }

    fn make_instr_resolve_obj_method() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ResolveObjMethod))
    }

    fn make_instr_resolve_cls_method(mode: ClsMethResolveOp) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ResolveClsMethod(mode)))
    }

    fn make_instr_await() -> Self {
        Self::make_instr(Instruct::IAsync(AsyncFunctions::Await))
    }

    fn make_instr_yield() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::Yield))
    }

    fn make_instr_yieldk() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::YieldK))
    }

    fn make_instr_createcont() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::CreateCont))
    }

    fn make_instr_awaitall(range: Option<(local::Type, isize)>) -> Self {
        Self::make_instr(Instruct::IAsync(AsyncFunctions::AwaitAll(range)))
    }

    fn make_instr_label(label: Label) -> Self {
        Self::make_instr(Instruct::ILabel(label))
    }

    fn make_instr_awaitall_list(unnamed_locals: Vec<local::Type>) -> Self {
        use local::Type::Unnamed;
        match unnamed_locals.split_first() {
            None => panic!("Expected at least one await"),
            Some((hd, tl)) => {
                if let Unnamed(hd_id) = hd {
                    let mut prev_id = hd_id;
                    for unnamed_local in tl.iter() {
                        match unnamed_local {
                            Unnamed(id) => {
                                assert_eq!(*prev_id + 1, *id);
                                prev_id = id;
                            }
                            _ => panic!("Expected unnamed local"),
                        }
                    }
                    Self::make_instr_awaitall(Some((
                        Unnamed(*hd_id),
                        unnamed_locals.len().try_into().unwrap(),
                    )))
                } else {
                    panic!("Expected unnamed local")
                }
            }
        }
    }

    fn make_instr_exit() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Exit))
    }

    fn make_instr_idx() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Idx))
    }

    fn make_instr_array_idx() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::ArrayIdx))
    }

    fn make_instr_fcallbuiltin(n: NumParams, un: NumParams, io: NumParams, s: String) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallBuiltin(n, un, io, s)))
    }

    fn make_instr_defcls(n: ClassNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefCls(n),
        ))
    }

    fn make_instr_defclsnop(n: ClassNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefClsNop(n),
        ))
    }

    fn make_instr_defrecord(n: ClassNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefRecord(n),
        ))
    }

    fn make_instr_deftypealias(n: ClassNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefTypeAlias(n),
        ))
    }

    fn make_instr_defcns(s: &'static str) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefCns(hhbc_id_rust::r#const::from_raw_string(s)),
        ))
    }

    fn make_instr_eval() -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::Eval,
        ))
    }

    fn make_instr_alias_cls(c1: String, c2: String) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::AliasCls(c1, c2),
        ))
    }

    fn make_instr_silence_start(local: local::Id) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Silence(
            local,
            OpSilence::Start,
        )))
    }

    fn make_instr_silence_end(local: local::Id) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Silence(
            local,
            OpSilence::End,
        )))
    }

    fn make_instr_cont_assign_delegate(iter: Iter) -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::ContAssignDelegate(
            iter,
        )))
    }

    fn make_instr_cont_enter_delegate() -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::ContEnterDelegate))
    }

    fn make_instr_yield_from_delegate(iter: Iter, l: Label) -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::YieldFromDelegate(
            iter, l,
        )))
    }

    fn make_instr_cont_unset_delegate_free(iter: Iter) -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::ContUnsetDelegate(
            FreeIterator::FreeIter,
            iter,
        )))
    }

    fn make_instr_cont_unset_delegate_ignore(iter: Iter) -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::ContUnsetDelegate(
            FreeIterator::IgnoreIter,
            iter,
        )))
    }

    fn make_instr_contcheck_check() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContCheck(
            CheckStarted::CheckStarted,
        )))
    }

    fn make_instr_contcheck_ignore() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContCheck(
            CheckStarted::IgnoreStarted,
        )))
    }

    fn make_instr_contenter() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContEnter))
    }

    fn make_instr_contraise() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContRaise))
    }

    fn make_instr_contvalid() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContValid))
    }

    fn make_instr_contcurrent() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContCurrent))
    }

    fn make_instr_contkey() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContKey))
    }

    fn make_instr_contgetreturn() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContGetReturn))
    }

    fn make_instr_trigger_sampled_error() -> Self {
        Self::make_instr_fcallbuiltin(3, 3, 0, String::from("trigger_sampled_error"))
    }

    fn make_instr_nativeimpl() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::NativeImpl))
    }

    fn create_try_catch(
        emitter: &mut Emitter,
        opt_done_label: Option<Label>,
        skip_throw: bool,
        try_instrs: Self,
        catch_instrs: Self,
    ) -> Self {
        let done_label = match opt_done_label {
            Some(l) => l,
            None => emitter.label_gen_mut().next_regular(),
        };
        Self::gather(vec![
            Self::make_instr(Instruct::ITry(InstructTry::TryCatchBegin)),
            try_instrs,
            Self::make_instr_jmp(done_label.clone()),
            Self::make_instr(Instruct::ITry(InstructTry::TryCatchMiddle)),
            catch_instrs,
            if skip_throw {
                Self::Empty
            } else {
                Self::make_instr(Instruct::IContFlow(InstructControlFlow::Throw))
            },
            Self::make_instr(Instruct::ITry(InstructTry::TryCatchEnd)),
            Self::make_instr_label(done_label.clone()),
        ])
    }

    fn get_or_put_label<'a>(
        emitter: &mut Emitter,
        name_label_map: &'a HashMap<String, Label>,
        name: String,
    ) -> (Label, &'a HashMap<String, Label>) {
        match name_label_map.get(&name) {
            Some(label) => (label.clone(), name_label_map),
            None => (emitter.label_gen_mut().next_regular(), name_label_map),
        }
    }

    fn rewrite_user_labels_instr<'a>(
        emitter: &mut Emitter,
        instruction: &Instruct,
        name_label_map: &'a HashMap<String, Label>,
    ) -> (Instruct, &'a HashMap<String, Label>) {
        use Instruct::*;
        let mut get_result = |x| Self::get_or_put_label(emitter, name_label_map, x);
        match instruction {
            IContFlow(InstructControlFlow::Jmp(Label::Named(name))) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    Instruct::IContFlow(InstructControlFlow::Jmp(label)),
                    name_label_map,
                )
            }
            IIterator(InstructIterator::IterBreak(Label::Named(name), iters)) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    IIterator(InstructIterator::IterBreak(label, iters.to_vec())),
                    name_label_map,
                )
            }
            IContFlow(InstructControlFlow::JmpNS(Label::Named(name))) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    Instruct::IContFlow(InstructControlFlow::JmpNS(label)),
                    name_label_map,
                )
            }
            IContFlow(InstructControlFlow::JmpZ(Label::Named(name))) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    Instruct::IContFlow(InstructControlFlow::JmpZ(label)),
                    name_label_map,
                )
            }
            IContFlow(InstructControlFlow::JmpNZ(Label::Named(name))) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    Instruct::IContFlow(InstructControlFlow::JmpNZ(label)),
                    name_label_map,
                )
            }
            ILabel(Label::Named(name)) => {
                let (label, name_label_map) = get_result(name.to_string());
                (ILabel(label), name_label_map)
            }
            i => (i.clone(), name_label_map),
        }
    }

    fn rewrite_user_labels_aux<'a>(
        emitter: &mut Emitter,
        instrseq: &Self,
        name_label_map: &'a HashMap<String, Label>,
    ) -> (Self, &'a HashMap<String, Label>) {
        match &instrseq {
            Instr::Empty => (Instr::Empty, name_label_map),
            Instr::One(instr) => {
                let (i, name_label_map) =
                    Self::rewrite_user_labels_instr(emitter, instr, name_label_map);
                (Self::make_instr(i), name_label_map)
            }
            Instr::Concat(instrseq) => {
                let folder = |(mut acc, map): (Vec<Self>, &'a HashMap<String, Label>),
                              seq: &Self| {
                    let (l, map) = Self::rewrite_user_labels_aux(emitter, seq, map);
                    acc.push(l);
                    (acc, map)
                };
                let (instrseq, name_label_map) =
                    instrseq.iter().fold((vec![], name_label_map), folder);
                (Instr::Concat(instrseq), name_label_map)
            }
            Instr::List(l) => {
                let folder = |(mut acc, map): (Vec<Instruct>, &'a HashMap<String, Label>),
                              instr: &Instruct| {
                    let (i, map) = Self::rewrite_user_labels_instr(emitter, instr, map);
                    acc.push(i);
                    (acc, map)
                };
                let (instrlst, name_label_map) = l.iter().fold((vec![], name_label_map), folder);
                (Instr::List(instrlst), name_label_map)
            }
        }
    }

    fn is_srcloc(instruction: &Instruct) -> bool {
        match instruction {
            Instruct::ISrcLoc(_) => true,
            _ => false,
        }
    }

    fn first(instrs: &Self) -> Option<&Instruct> {
        match instrs {
            Self::Empty => None,
            Self::One(i) => {
                if Self::is_srcloc(i) {
                    None
                } else {
                    Some(i)
                }
            }
            Self::List(l) => match l.iter().find(|&i| !Self::is_srcloc(i)) {
                Some(i) => Some(i),
                None => None,
            },
            Self::Concat(l) => l.iter().find_map(Self::first),
        }
    }

    fn is_empty(instrs: &Self) -> bool {
        match instrs {
            Self::Empty => true,
            Self::One(i) => Self::is_srcloc(i),
            Self::List(l) => l.is_empty() || l.iter().all(Self::is_srcloc),
            Self::Concat(l) => l.iter().all(Self::is_empty),
        }
    }
}

pub mod instr_seq {
    use crate::Instr;
    use hhbc_ast_rust::Instruct;

    pub fn flat_map<F>(instrseq: &Instr, f: &mut F) -> Instr
    where
        F: FnMut(&Instruct) -> Vec<Instruct>,
    {
        match instrseq {
            Instr::Empty => Instr::Empty,
            Instr::One(instr) => match &f(&instr)[..] {
                [] => Instr::Empty,
                [x] => Instr::make_instr(x.clone()),
                xs => Instr::List(xs.to_vec()),
            },
            Instr::List(instr_lst) => {
                let newlst = instr_lst.iter().flat_map(|x| f(x)).collect::<Vec<_>>();
                Instr::List(newlst)
            }
            Instr::Concat(instrseq_lst) => {
                let newlst = instrseq_lst
                    .iter()
                    .map(|x| flat_map(x, f))
                    .collect::<Vec<_>>();
                Instr::Concat(newlst)
            }
        }
    }

    pub fn flat_map_seq<F>(instrseq: &Instr, f: &mut F) -> Instr
    where
        F: FnMut(&Instruct) -> Instr,
    {
        match instrseq {
            Instr::Empty => Instr::Empty,
            Instr::One(instr) => f(instr),
            Instr::List(instr_lst) => {
                Instr::Concat(instr_lst.iter().map(|x| f(x)).collect::<Vec<_>>())
            }
            Instr::Concat(instrseq_lst) => Instr::Concat(
                instrseq_lst
                    .iter()
                    .map(|x| flat_map_seq(x, f))
                    .collect::<Vec<_>>(),
            ),
        }
    }

    pub fn fold_left<F, A>(instrseq: &Instr, f: &mut F, init: A) -> A
    where
        F: FnMut(A, &Instruct) -> A,
    {
        match instrseq {
            Instr::Empty => init,
            Instr::One(x) => f(init, x),
            Instr::List(instr_lst) => instr_lst.iter().fold(init, f),
            Instr::Concat(instrseq_lst) => instrseq_lst
                .iter()
                .fold(init, |acc, x| fold_left(x, f, acc)),
        }
    }

    pub fn filter_map<F>(instrseq: &Instr, f: &mut F) -> Instr
    where
        F: FnMut(&Instruct) -> Option<Instruct>,
    {
        match instrseq {
            Instr::Empty => Instr::Empty,
            Instr::One(x) => match f(x) {
                Some(x) => Instr::make_instr(x.clone()),
                None => Instr::Empty,
            },
            Instr::List(instr_lst) => {
                Instr::List(instr_lst.iter().filter_map(f).collect::<Vec<_>>())
            }
            Instr::Concat(instrseq_lst) => Instr::Concat(
                instrseq_lst
                    .iter()
                    .map(|x| filter_map(x, f))
                    .collect::<Vec<_>>(),
            ),
        }
    }

    pub fn map_mut<F>(instrseq: &mut Instr, f: &mut F)
    where
        F: FnMut(&mut Instruct),
    {
        match instrseq {
            Instr::Empty => (),
            Instr::One(x) => f(x),
            Instr::List(instr_lst) => instr_lst.iter_mut().for_each(|x| f(x)),
            Instr::Concat(instrseq_lst) => instrseq_lst.iter_mut().for_each(|x| map_mut(x, f)),
        }
    }

    pub fn map<F>(instrseq: &Instr, f: &mut F) -> Instr
    where
        F: FnMut(Instruct) -> Instruct,
    {
        filter_map(instrseq, &mut |x| Some(f(x.clone())))
    }
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::print;
use ffi::{Maybe, Str};
use hash::HashSet;
use hhbc_ast::{
    AdataId, BareThisOp, ClassId, ClassNum, CollectionType, ConstId, ContCheckOp, FCallArgs,
    FatalOp, FunctionId, IncDecOp, InitPropOp, IsLogAsDynamicCallOp, IsTypeOp, IterArgs,
    LocalRange, MOpMode, MemberKey, MethodId, NumParams, OODeclExistsOp, ObjMethodOp, Opcode,
    ParamId, PropId, QueryMOp, ReadonlyOp, SetOpOp, SetRangeOp, SilenceOp, SpecialClsRef,
    StackIndex, SwitchKind, TypeStructResolveOp,
};
use hhbc_string_utils::float;
use iterator::IterId;
use label::Label;
use local::Local;
use print_opcode::PrintOpcodeTypes;
use std::io::{Error, ErrorKind, Result, Write};

pub(crate) struct PrintOpcode<'a, 'b, 'c> {
    pub(crate) opcode: &'b Opcode<'a>,
    pub(crate) dv_labels: &'b HashSet<Label>,
    pub(crate) phantom: std::marker::PhantomData<&'c i8>,
}

impl<'a, 'b, 'c> PrintOpcode<'a, 'b, 'c> {
    pub(crate) fn new(opcode: &'b Opcode<'a>, dv_labels: &'b HashSet<Label>) -> Self {
        Self {
            opcode,
            dv_labels,
            phantom: Default::default(),
        }
    }

    fn get_opcode(&self) -> &'b Opcode<'a> {
        self.opcode
    }

    fn print_branch_labels(&self, w: &mut dyn Write, labels: &[Label]) -> Result<()> {
        w.write_all(b"<")?;
        for (i, label) in labels.iter().enumerate() {
            if i != 0 {
                w.write_all(b" ")?;
            }
            self.print_label(w, label)?;
        }
        w.write_all(b">")
    }

    fn print_fcall_args(&self, w: &mut dyn Write, args: &FCallArgs<'_>) -> Result<()> {
        print::print_fcall_args(w, args, self.dv_labels)
    }

    fn print_label(&self, w: &mut dyn Write, label: &Label) -> Result<()> {
        print::print_label(w, label, self.dv_labels)
    }

    fn print_label2(&self, w: &mut dyn Write, [label1, label2]: &[Label; 2]) -> Result<()> {
        self.print_label(w, label1)?;
        w.write_all(b" ")?;
        self.print_label(w, label2)
    }

    fn print_s_switch(
        &self,
        w: &mut dyn Write,
        cases: &[Str<'_>],
        targets: &[Label],
    ) -> Result<()> {
        if cases.len() != targets.len() {
            return Err(Error::new(
                ErrorKind::Other,
                "sswitch cases and targets must match length",
            ));
        }

        let mut iter = cases.iter().zip(targets).rev();
        let (_, last_target) = if let Some(last) = iter.next() {
            last
        } else {
            return Err(Error::new(
                ErrorKind::Other,
                "sswitch should have at least one case",
            ));
        };
        let iter = iter.rev();

        w.write_all(b"SSwitch <")?;
        for (case, target) in iter {
            print_quoted_str(w, case)?;
            w.write_all(b":")?;
            self.print_label(w, target)?;
            w.write_all(b" ")?;
        }

        w.write_all(b"-:")?;
        self.print_label(w, last_target)?;
        w.write_all(b">")
    }
}

macro_rules! print_with_display {
    ($func_name: ident, $ty: ty) => {
        fn $func_name(w: &mut dyn Write, arg: &$ty) -> Result<()> {
            write!(w, "{}", arg)
        }
    };
}

print_with_display!(print_class_num, ClassNum);
print_with_display!(print_num_params, NumParams);
print_with_display!(print_stack_index, StackIndex);

macro_rules! print_with_debug {
    ($vis: vis $func_name: ident, $ty: ty) => {
        $vis fn $func_name(w: &mut dyn Write, arg: &$ty) -> Result<()> {
            write!(w, "{:?}", arg)
        }
    };
}

print_with_debug!(print_bare_this_op, BareThisOp);
print_with_debug!(print_collection_type, CollectionType);
print_with_debug!(print_cont_check_op, ContCheckOp);
print_with_debug!(print_fatal_op, FatalOp);
print_with_debug!(print_inc_dec_op, IncDecOp);
print_with_debug!(print_init_prop_op, InitPropOp);
print_with_debug!(print_is_log_as_dynamic_call_op, IsLogAsDynamicCallOp);
print_with_debug!(print_is_type_op, IsTypeOp);
print_with_debug!(print_m_op_mode, MOpMode);
print_with_debug!(print_obj_method_op, ObjMethodOp);
print_with_debug!(print_oo_decl_exists_op, OODeclExistsOp);
print_with_debug!(print_query_m_op, QueryMOp);
print_with_debug!(print_readonly_op, ReadonlyOp);
print_with_debug!(print_set_op_op, SetOpOp);
print_with_debug!(print_set_range_op, SetRangeOp);
print_with_debug!(print_silence_op, SilenceOp);
print_with_debug!(print_special_cls_ref, SpecialClsRef);
print_with_debug!(print_switch_kind, SwitchKind);
print_with_debug!(print_type_struct_resolve_op, TypeStructResolveOp);

fn print_adata_id(w: &mut dyn Write, id: &AdataId<'_>) -> Result<()> {
    w.write_all(b"@")?;
    print_str(w, id)
}

fn print_class_id(w: &mut dyn Write, id: &ClassId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_const_id(w: &mut dyn Write, id: &ConstId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_double(w: &mut dyn Write, d: f64) -> Result<()> {
    write!(w, "{}", float::to_string(d))
}

fn print_function_id(w: &mut dyn Write, id: &FunctionId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_iter_args(w: &mut dyn Write, iter_args: &IterArgs<'_>) -> Result<()> {
    print_iterator_id(w, &iter_args.iter_id)?;
    w.write_all(b" ")?;
    match &iter_args.key_id {
        Maybe::Nothing => w.write_all(b"NK")?,
        Maybe::Just(k) => {
            w.write_all(b"K:")?;
            print_local(w, k)?;
        }
    };
    w.write_all(b" ")?;
    w.write_all(b"V:")?;
    print_local(w, &iter_args.val_id)
}

fn print_iterator_id(w: &mut dyn Write, i: &IterId) -> Result<()> {
    write!(w, "{}", i)
}

fn print_local(w: &mut dyn Write, local: &Local<'_>) -> Result<()> {
    match local {
        Local::Unnamed(id) => write!(w, "_{}", id),
        Local::Named(id) => w.write_all(id),
    }
}

fn print_local_range(w: &mut dyn Write, locrange: &LocalRange) -> Result<()> {
    write!(w, "L:{}+{}", locrange.start, locrange.len)
}

fn print_member_key(w: &mut dyn Write, mk: &MemberKey<'_>) -> Result<()> {
    use MemberKey as M;
    match mk {
        M::EC(si, op) => {
            w.write_all(b"EC:")?;
            print_stack_index(w, si)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::EL(local, op) => {
            w.write_all(b"EL:")?;
            print_local(w, local)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::ET(s, op) => {
            w.write_all(b"ET:")?;
            print_quoted_str(w, s)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::EI(i, op) => {
            write!(w, "EI:{} ", i)?;
            print_readonly_op(w, op)
        }
        M::PC(si, op) => {
            w.write_all(b"PC:")?;
            print_stack_index(w, si)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::PL(local, op) => {
            w.write_all(b"PL:")?;
            print_local(w, local)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::PT(id, op) => {
            w.write_all(b"PT:")?;
            print_prop_id(w, id)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::QT(id, op) => {
            w.write_all(b"QT:")?;
            print_prop_id(w, id)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::W => w.write_all(b"W"),
    }
}

fn print_method_id(w: &mut dyn Write, id: &MethodId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_param_id(w: &mut dyn Write, param_id: &ParamId<'_>) -> Result<()> {
    match param_id {
        ParamId::ParamUnnamed(i) => w.write_all(i.to_string().as_bytes()),
        ParamId::ParamNamed(s) => w.write_all(s),
    }
}

pub(crate) fn print_prop_id(w: &mut dyn Write, id: &PropId<'_>) -> Result<()> {
    print_quoted_str(w, &id.as_ffi_str())
}

fn print_quoted_str(w: &mut dyn Write, s: &ffi::Str<'_>) -> Result<()> {
    w.write_all(b"\"")?;
    w.write_all(&escaper::escape_bstr(s.as_bstr()))?;
    w.write_all(b"\"")
}

fn print_shape_fields(w: &mut dyn Write, keys: &[ffi::Str<'_>]) -> Result<()> {
    w.write_all(b"<")?;
    for (i, key) in keys.iter().enumerate() {
        if i != 0 {
            w.write_all(b" ")?;
        }
        print_quoted_str(w, key)?;
    }
    w.write_all(b">")
}

fn print_str(w: &mut dyn Write, s: &ffi::Str<'_>) -> Result<()> {
    use bstr::ByteSlice;
    w.write_all(s.as_bytes())
}

impl<'a, 'b, 'c> PrintOpcodeTypes for PrintOpcode<'a, 'b, 'c> {
    type Write = dyn Write + 'c;
    type Error = Error;
}

impl<'a, 'b, 'c> PrintOpcode<'a, 'b, 'c> {
    pub(crate) fn print_opcode(
        &self,
        w: &mut <Self as PrintOpcodeTypes>::Write,
    ) -> std::result::Result<(), <Self as PrintOpcodeTypes>::Error> {
        match self.get_opcode() {
            Opcode::AKExists => w.write_all(b"AKExists"),
            Opcode::Add => w.write_all(b"Add"),
            Opcode::AddElemC => w.write_all(b"AddElemC"),
            Opcode::AddNewElemC => w.write_all(b"AddNewElemC"),
            Opcode::AddO => w.write_all(b"AddO"),
            Opcode::ArrayIdx => w.write_all(b"ArrayIdx"),
            Opcode::ArrayMarkLegacy => w.write_all(b"ArrayMarkLegacy"),
            Opcode::ArrayUnmarkLegacy => w.write_all(b"ArrayUnmarkLegacy"),
            Opcode::AssertRATL(local, s) => {
                w.write_all(b"AssertRATL ")?;
                print_local(w, local)?;
                w.write_all(b" ")?;
                print_str(w, s)
            }
            Opcode::AssertRATStk(n, s) => {
                w.write_all(b"AssertRATStk ")?;
                print_stack_index(w, n)?;
                w.write_all(b" ")?;
                print_str(w, s)
            }
            Opcode::Await => w.write_all(b"Await"),
            Opcode::AwaitAll(range) => {
                w.write_all(b"AwaitAll ")?;
                print_local_range(w, range)
            }
            Opcode::BareThis(op) => {
                w.write_all(b"BareThis ")?;
                print_bare_this_op(w, op)
            }
            Opcode::BaseC(si, m) => {
                w.write_all(b"BaseC ")?;
                print_stack_index(w, si)?;
                w.write_all(b" ")?;
                print_m_op_mode(w, m)
            }
            Opcode::BaseGC(si, m) => {
                w.write_all(b"BaseGC ")?;
                print_stack_index(w, si)?;
                w.write_all(b" ")?;
                print_m_op_mode(w, m)
            }
            Opcode::BaseGL(id, m) => {
                w.write_all(b"BaseGL ")?;
                print_local(w, id)?;
                w.write_all(b" ")?;
                print_m_op_mode(w, m)
            }
            Opcode::BaseH => w.write_all(b"BaseH"),
            Opcode::BaseL(id, m, op) => {
                w.write_all(b"BaseL ")?;
                print_local(w, id)?;
                w.write_all(b" ")?;
                print_m_op_mode(w, m)?;
                w.write_all(b" ")?;
                print_readonly_op(w, op)
            }
            Opcode::BaseSC(si1, si2, m, op) => {
                w.write_all(b"BaseSC ")?;
                print_stack_index(w, si1)?;
                w.write_all(b" ")?;
                print_stack_index(w, si2)?;
                w.write_all(b" ")?;
                print_m_op_mode(w, m)?;
                w.write_all(b" ")?;
                print_readonly_op(w, op)
            }
            Opcode::BitAnd => w.write_all(b"BitAnd"),
            Opcode::BitNot => w.write_all(b"BitNot"),
            Opcode::BitOr => w.write_all(b"BitOr"),
            Opcode::BitXor => w.write_all(b"BitXor"),
            Opcode::BreakTraceHint => w.write_all(b"BreakTraceHint"),
            Opcode::CGetCUNop => w.write_all(b"CGetCUNop"),
            Opcode::CGetG => w.write_all(b"CGetG"),
            Opcode::CGetL(id) => {
                w.write_all(b"CGetL ")?;
                print_local(w, id)
            }
            Opcode::CGetL2(id) => {
                w.write_all(b"CGetL2 ")?;
                print_local(w, id)
            }
            Opcode::CGetQuietL(id) => {
                w.write_all(b"CGetQuietL ")?;
                print_local(w, id)
            }
            Opcode::CGetS(op) => {
                w.write_all(b"CGetS ")?;
                print_readonly_op(w, op)
            }
            Opcode::CUGetL(id) => {
                w.write_all(b"CUGetL ")?;
                print_local(w, id)
            }
            Opcode::CastBool => w.write_all(b"CastBool"),
            Opcode::CastDict => w.write_all(b"CastDict"),
            Opcode::CastDouble => w.write_all(b"CastDouble"),
            Opcode::CastInt => w.write_all(b"CastInt"),
            Opcode::CastKeyset => w.write_all(b"CastKeyset"),
            Opcode::CastString => w.write_all(b"CastString"),
            Opcode::CastVec => w.write_all(b"CastVec"),
            Opcode::ChainFaults => w.write_all(b"ChainFaults"),
            Opcode::CheckProp(id) => {
                w.write_all(b"CheckProp ")?;
                print_prop_id(w, id)
            }
            Opcode::CheckReifiedGenericMismatch => w.write_all(b"CheckReifiedGenericMismatch"),
            Opcode::CheckThis => w.write_all(b"CheckThis"),
            Opcode::ClassGetC => w.write_all(b"ClassGetC"),
            Opcode::ClassGetTS => w.write_all(b"ClassGetTS"),
            Opcode::ClassName => w.write_all(b"ClassName"),
            Opcode::Clone => w.write_all(b"Clone"),
            Opcode::ClsCns(id) => {
                w.write_all(b"ClsCns ")?;
                print_const_id(w, id)
            }
            Opcode::ClsCnsD(const_id, cid) => {
                w.write_all(b"ClsCnsD ")?;
                print_const_id(w, const_id)?;
                w.write_all(b" ")?;
                print_class_id(w, cid)
            }
            Opcode::ClsCnsL(id) => {
                w.write_all(b"ClsCnsL ")?;
                print_local(w, id)
            }
            Opcode::Cmp => w.write_all(b"Cmp"),
            Opcode::CnsE(id) => {
                w.write_all(b"CnsE ")?;
                print_const_id(w, id)
            }
            Opcode::ColFromArray(ct) => {
                w.write_all(b"ColFromArray ")?;
                print_collection_type(w, ct)
            }
            Opcode::CombineAndResolveTypeStruct(n) => {
                w.write_all(b"CombineAndResolveTypeStruct ")?;
                write!(w, "{}", n)
            }
            Opcode::Concat => w.write_all(b"Concat"),
            Opcode::ConcatN(n) => {
                w.write_all(b"ConcatN ")?;
                write!(w, "{}", n)
            }
            Opcode::ContCheck(subop1) => {
                w.write_all(b"ContCheck ")?;
                print_cont_check_op(w, subop1)
            }
            Opcode::ContCurrent => w.write_all(b"ContCurrent"),
            Opcode::ContEnter => w.write_all(b"ContEnter"),
            Opcode::ContGetReturn => w.write_all(b"ContGetReturn"),
            Opcode::ContKey => w.write_all(b"ContKey"),
            Opcode::ContRaise => w.write_all(b"ContRaise"),
            Opcode::ContValid => w.write_all(b"ContValid"),
            Opcode::CreateCl(n, cid) => {
                w.write_all(b"CreateCl ")?;
                print_num_params(w, n)?;
                w.write_all(b" ")?;
                print_class_num(w, cid)
            }
            Opcode::CreateCont => w.write_all(b"CreateCont"),
            Opcode::DblAsBits => w.write_all(b"DblAsBits"),
            Opcode::Dict(id) => {
                w.write_all(b"Dict ")?;
                print_adata_id(w, id)
            }
            Opcode::Dim(m, mk) => {
                w.write_all(b"Dim ")?;
                print_m_op_mode(w, m)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::Dir => w.write_all(b"Dir"),
            Opcode::Div => w.write_all(b"Div"),
            Opcode::Double(d) => {
                w.write_all(b"Double ")?;
                print_double(w, *d)
            }
            Opcode::Dup => w.write_all(b"Dup"),
            Opcode::EntryNop => w.write_all(b"EntryNop"),
            Opcode::Eq => w.write_all(b"Eq"),
            Opcode::Eval => w.write_all(b"Eval"),
            Opcode::Exit => w.write_all(b"Exit"),
            Opcode::FCallClsMethod(fcall_args, hint, log) => {
                w.write_all(b"FCallClsMethod ")?;
                self.print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_quoted_str(w, hint)?;
                w.write_all(b" ")?;
                print_is_log_as_dynamic_call_op(w, log)
            }
            Opcode::FCallClsMethodD(fcall_args, hint, class, method) => {
                w.write_all(b"FCallClsMethodD ")?;
                self.print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_quoted_str(w, hint)?;
                w.write_all(b" ")?;
                print_class_id(w, class)?;
                w.write_all(b" ")?;
                print_method_id(w, method)
            }
            Opcode::FCallClsMethodS(fcall_args, hint, clsref) => {
                w.write_all(b"FCallClsMethodS ")?;
                self.print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_quoted_str(w, hint)?;
                w.write_all(b" ")?;
                print_special_cls_ref(w, clsref)
            }
            Opcode::FCallClsMethodSD(fcall_args, hint, clsref, method) => {
                w.write_all(b"FCallClsMethodSD ")?;
                self.print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_quoted_str(w, hint)?;
                w.write_all(b" ")?;
                print_special_cls_ref(w, clsref)?;
                w.write_all(b" ")?;
                print_method_id(w, method)
            }
            Opcode::FCallCtor(fcall_args, hint) => {
                w.write_all(b"FCallCtor ")?;
                self.print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_quoted_str(w, hint)
            }
            Opcode::FCallFunc(fcall_args) => {
                w.write_all(b"FCallFunc ")?;
                self.print_fcall_args(w, fcall_args)
            }
            Opcode::FCallFuncD(fcall_args, func) => {
                w.write_all(b"FCallFuncD ")?;
                self.print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_function_id(w, func)
            }
            Opcode::FCallObjMethod(fcall_args, hint, flavor) => {
                w.write_all(b"FCallObjMethod ")?;
                self.print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_quoted_str(w, hint)?;
                w.write_all(b" ")?;
                print_obj_method_op(w, flavor)
            }
            Opcode::FCallObjMethodD(fcall_args, hint, flavor, method) => {
                w.write_all(b"FCallObjMethodD ")?;
                self.print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_quoted_str(w, hint)?;
                w.write_all(b" ")?;
                print_obj_method_op(w, flavor)?;
                w.write_all(b" ")?;
                print_method_id(w, method)
            }
            Opcode::False => w.write_all(b"False"),
            Opcode::Fatal(fatal_op) => {
                w.write_all(b"Fatal ")?;
                print_fatal_op(w, fatal_op)
            }
            Opcode::File => w.write_all(b"File"),
            Opcode::FuncCred => w.write_all(b"FuncCred"),
            Opcode::GetMemoKeyL(local) => {
                w.write_all(b"GetMemoKeyL ")?;
                print_local(w, local)
            }
            Opcode::Gt => w.write_all(b"Gt"),
            Opcode::Gte => w.write_all(b"Gte"),
            Opcode::Idx => w.write_all(b"Idx"),
            Opcode::IncDecG(op) => {
                w.write_all(b"IncDecG ")?;
                print_inc_dec_op(w, op)
            }
            Opcode::IncDecL(id, op) => {
                w.write_all(b"IncDecL ")?;
                print_local(w, id)?;
                w.write_all(b" ")?;
                print_inc_dec_op(w, op)
            }
            Opcode::IncDecM(i, op, mk) => {
                w.write_all(b"IncDecM ")?;
                print_stack_index(w, i)?;
                w.write_all(b" ")?;
                print_inc_dec_op(w, op)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::IncDecS(op) => {
                w.write_all(b"IncDecS ")?;
                print_inc_dec_op(w, op)
            }
            Opcode::Incl => w.write_all(b"Incl"),
            Opcode::InclOnce => w.write_all(b"InclOnce"),
            Opcode::InitProp(id, op) => {
                w.write_all(b"InitProp ")?;
                print_prop_id(w, id)?;
                w.write_all(b" ")?;
                print_init_prop_op(w, op)
            }
            Opcode::InstanceOf => w.write_all(b"InstanceOf"),
            Opcode::InstanceOfD(id) => {
                w.write_all(b"InstanceOfD ")?;
                print_class_id(w, id)
            }
            Opcode::Int(i) => {
                w.write_all(b"Int ")?;
                write!(w, "{}", i)
            }
            Opcode::IsLateBoundCls => w.write_all(b"IsLateBoundCls"),
            Opcode::IsTypeC(op) => {
                w.write_all(b"IsTypeC ")?;
                print_is_type_op(w, op)
            }
            Opcode::IsTypeL(local, op) => {
                w.write_all(b"IsTypeL ")?;
                print_local(w, local)?;
                w.write_all(b" ")?;
                print_is_type_op(w, op)
            }
            Opcode::IsTypeStructC(op) => {
                w.write_all(b"IsTypeStructC ")?;
                print_type_struct_resolve_op(w, op)
            }
            Opcode::IsUnsetL(local) => {
                w.write_all(b"IsUnsetL ")?;
                print_local(w, local)
            }
            Opcode::IssetG => w.write_all(b"IssetG"),
            Opcode::IssetL(local) => {
                w.write_all(b"IssetL ")?;
                print_local(w, local)
            }
            Opcode::IssetS => w.write_all(b"IssetS"),
            Opcode::IterFree(id) => {
                w.write_all(b"IterFree ")?;
                print_iterator_id(w, id)
            }
            Opcode::IterInit(iter_args, label) => {
                w.write_all(b"IterInit ")?;
                print_iter_args(w, iter_args)?;
                w.write_all(b" ")?;
                self.print_label(w, label)
            }
            Opcode::IterNext(iter_args, label) => {
                w.write_all(b"IterNext ")?;
                print_iter_args(w, iter_args)?;
                w.write_all(b" ")?;
                self.print_label(w, label)
            }
            Opcode::Jmp(l) => {
                w.write_all(b"Jmp ")?;
                self.print_label(w, l)
            }
            Opcode::JmpNS(l) => {
                w.write_all(b"JmpNS ")?;
                self.print_label(w, l)
            }
            Opcode::JmpNZ(l) => {
                w.write_all(b"JmpNZ ")?;
                self.print_label(w, l)
            }
            Opcode::JmpZ(l) => {
                w.write_all(b"JmpZ ")?;
                self.print_label(w, l)
            }
            Opcode::Keyset(id) => {
                w.write_all(b"Keyset ")?;
                print_adata_id(w, id)
            }
            Opcode::LIterFree(iter1, loc2) => {
                w.write_all(b"LIterFree ")?;
                print_iterator_id(w, iter1)?;
                w.write_all(b" ")?;
                print_local(w, loc2)
            }
            Opcode::LIterInit(ita, loc2, target3) => {
                w.write_all(b"LIterInit ")?;
                print_iter_args(w, ita)?;
                w.write_all(b" ")?;
                print_local(w, loc2)?;
                w.write_all(b" ")?;
                self.print_label(w, target3)
            }
            Opcode::LIterNext(ita, loc2, target3) => {
                w.write_all(b"LIterNext ")?;
                print_iter_args(w, ita)?;
                w.write_all(b" ")?;
                print_local(w, loc2)?;
                w.write_all(b" ")?;
                self.print_label(w, target3)
            }
            Opcode::LateBoundCls => w.write_all(b"LateBoundCls"),
            Opcode::LazyClass(id) => {
                w.write_all(b"LazyClass ")?;
                print_class_id(w, id)
            }
            Opcode::LazyClassFromClass => w.write_all(b"LazyClassFromClass"),
            Opcode::LockObj => w.write_all(b"LockObj"),
            Opcode::Lt => w.write_all(b"Lt"),
            Opcode::Lte => w.write_all(b"Lte"),
            Opcode::MemoGet(label, range) => {
                w.write_all(b"MemoGet ")?;
                self.print_label(w, label)?;
                w.write_all(b" ")?;
                print_local_range(w, range)
            }
            Opcode::MemoGetEager(target1, range) => {
                w.write_all(b"MemoGetEager ")?;
                self.print_label2(w, target1)?;
                w.write_all(b" ")?;
                print_local_range(w, range)
            }
            Opcode::MemoSet(range) => {
                w.write_all(b"MemoSet ")?;
                print_local_range(w, range)
            }
            Opcode::MemoSetEager(range) => {
                w.write_all(b"MemoSetEager ")?;
                print_local_range(w, range)
            }
            Opcode::Method => w.write_all(b"Method"),
            Opcode::Mod => w.write_all(b"Mod"),
            Opcode::Mul => w.write_all(b"Mul"),
            Opcode::MulO => w.write_all(b"MulO"),
            Opcode::NSame => w.write_all(b"NSame"),
            Opcode::NativeImpl => w.write_all(b"NativeImpl"),
            Opcode::Neq => w.write_all(b"Neq"),
            Opcode::NewCol(ct) => {
                w.write_all(b"NewCol ")?;
                print_collection_type(w, ct)
            }
            Opcode::NewDictArray(i) => {
                w.write_all(b"NewDictArray ")?;
                write!(w, "{}", i)
            }
            Opcode::NewKeysetArray(i) => {
                w.write_all(b"NewKeysetArray ")?;
                write!(w, "{}", i)
            }
            Opcode::NewObj => w.write_all(b"NewObj"),
            Opcode::NewObjD(cid) => {
                w.write_all(b"NewObjD ")?;
                print_class_id(w, cid)
            }
            Opcode::NewObjR => w.write_all(b"NewObjR"),
            Opcode::NewObjRD(cid) => {
                w.write_all(b"NewObjRD ")?;
                print_class_id(w, cid)
            }
            Opcode::NewObjS(r) => {
                w.write_all(b"NewObjS ")?;
                print_special_cls_ref(w, r)
            }
            Opcode::NewPair => w.write_all(b"NewPair"),
            Opcode::NewStructDict(l) => {
                w.write_all(b"NewStructDict ")?;
                print_shape_fields(w, l)
            }
            Opcode::NewVec(i) => {
                w.write_all(b"NewVec ")?;
                write!(w, "{}", i)
            }
            Opcode::Nop => w.write_all(b"Nop"),
            Opcode::Not => w.write_all(b"Not"),
            Opcode::Null => w.write_all(b"Null"),
            Opcode::NullUninit => w.write_all(b"NullUninit"),
            Opcode::OODeclExists(k) => {
                w.write_all(b"OODeclExists ")?;
                print_oo_decl_exists_op(w, k)
            }
            Opcode::ParentCls => w.write_all(b"ParentCls"),
            Opcode::PopC => w.write_all(b"PopC"),
            Opcode::PopL(id) => {
                w.write_all(b"PopL ")?;
                print_local(w, id)
            }
            Opcode::PopU => w.write_all(b"PopU"),
            Opcode::PopU2 => w.write_all(b"PopU2"),
            Opcode::Pow => w.write_all(b"Pow"),
            Opcode::Print => w.write_all(b"Print"),
            Opcode::PushL(id) => {
                w.write_all(b"PushL ")?;
                print_local(w, id)
            }
            Opcode::QueryM(n, op, mk) => {
                w.write_all(b"QueryM ")?;
                print_stack_index(w, n)?;
                w.write_all(b" ")?;
                print_query_m_op(w, op)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::RaiseClassStringConversionWarning => {
                w.write_all(b"RaiseClassStringConversionWarning")
            }
            Opcode::RecordReifiedGeneric => w.write_all(b"RecordReifiedGeneric"),
            Opcode::Req => w.write_all(b"Req"),
            Opcode::ReqDoc => w.write_all(b"ReqDoc"),
            Opcode::ReqOnce => w.write_all(b"ReqOnce"),
            Opcode::ResolveClass(id) => {
                w.write_all(b"ResolveClass ")?;
                print_class_id(w, id)
            }
            Opcode::ResolveClsMethod(mid) => {
                w.write_all(b"ResolveClsMethod ")?;
                print_method_id(w, mid)
            }
            Opcode::ResolveClsMethodD(cid, mid) => {
                w.write_all(b"ResolveClsMethodD ")?;
                print_class_id(w, cid)?;
                w.write_all(b" ")?;
                print_method_id(w, mid)
            }
            Opcode::ResolveClsMethodS(r, mid) => {
                w.write_all(b"ResolveClsMethodS ")?;
                print_special_cls_ref(w, r)?;
                w.write_all(b" ")?;
                print_method_id(w, mid)
            }
            Opcode::ResolveFunc(id) => {
                w.write_all(b"ResolveFunc ")?;
                print_function_id(w, id)
            }
            Opcode::ResolveMethCaller(id) => {
                w.write_all(b"ResolveMethCaller ")?;
                print_function_id(w, id)
            }
            Opcode::ResolveRClsMethod(mid) => {
                w.write_all(b"ResolveRClsMethod ")?;
                print_method_id(w, mid)
            }
            Opcode::ResolveRClsMethodD(cid, mid) => {
                w.write_all(b"ResolveRClsMethodD ")?;
                print_class_id(w, cid)?;
                w.write_all(b" ")?;
                print_method_id(w, mid)
            }
            Opcode::ResolveRClsMethodS(r, mid) => {
                w.write_all(b"ResolveRClsMethodS ")?;
                print_special_cls_ref(w, r)?;
                w.write_all(b" ")?;
                print_method_id(w, mid)
            }
            Opcode::ResolveRFunc(id) => {
                w.write_all(b"ResolveRFunc ")?;
                print_function_id(w, id)
            }
            Opcode::RetC => w.write_all(b"RetC"),
            Opcode::RetCSuspended => w.write_all(b"RetCSuspended"),
            Opcode::RetM(p) => {
                w.write_all(b"RetM ")?;
                print_stack_index(w, p)
            }
            Opcode::SSwitch { cases, targets } => {
                self.print_s_switch(w, cases.as_ref(), targets.as_ref())
            }
            Opcode::Same => w.write_all(b"Same"),
            Opcode::Select => w.write_all(b"Select"),
            Opcode::SelfCls => w.write_all(b"SelfCls"),
            Opcode::SetG => w.write_all(b"SetG"),
            Opcode::SetImplicitContextByValue => w.write_all(b"SetImplicitContextByValue"),
            Opcode::SetL(local) => {
                w.write_all(b"SetL ")?;
                print_local(w, local)
            }
            Opcode::SetM(i, mk) => {
                w.write_all(b"SetM ")?;
                print_stack_index(w, i)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::SetOpG(op) => {
                w.write_all(b"SetOpG ")?;
                print_set_op_op(w, op)
            }
            Opcode::SetOpL(id, op) => {
                w.write_all(b"SetOpL ")?;
                print_local(w, id)?;
                w.write_all(b" ")?;
                print_set_op_op(w, op)
            }
            Opcode::SetOpM(i, op, mk) => {
                w.write_all(b"SetOpM ")?;
                print_stack_index(w, i)?;
                w.write_all(b" ")?;
                print_set_op_op(w, op)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::SetOpS(op) => {
                w.write_all(b"SetOpS ")?;
                print_set_op_op(w, op)
            }
            Opcode::SetRangeM(i, s, op) => {
                w.write_all(b"SetRangeM ")?;
                print_stack_index(w, i)?;
                w.write_all(b" ")?;
                write!(w, "{}", s)?;
                w.write_all(b" ")?;
                print_set_range_op(w, op)
            }
            Opcode::SetS(op) => {
                w.write_all(b"SetS ")?;
                print_readonly_op(w, op)
            }
            Opcode::Shl => w.write_all(b"Shl"),
            Opcode::Shr => w.write_all(b"Shr"),
            Opcode::Silence(local, op) => {
                w.write_all(b"Silence ")?;
                print_local(w, local)?;
                w.write_all(b" ")?;
                print_silence_op(w, op)
            }
            Opcode::String(s) => {
                w.write_all(b"String ")?;
                print_quoted_str(w, s)
            }
            Opcode::Sub => w.write_all(b"Sub"),
            Opcode::SubO => w.write_all(b"SubO"),
            Opcode::Switch(kind, base, targets) => {
                w.write_all(b"Switch ")?;
                print_switch_kind(w, kind)?;
                w.write_all(b" ")?;
                write!(w, "{}", base)?;
                w.write_all(b" ")?;
                self.print_branch_labels(w, targets.as_ref())
            }
            Opcode::This => w.write_all(b"This"),
            Opcode::Throw => w.write_all(b"Throw"),
            Opcode::ThrowAsTypeStructException => w.write_all(b"ThrowAsTypeStructException"),
            Opcode::ThrowNonExhaustiveSwitch => w.write_all(b"ThrowNonExhaustiveSwitch"),
            Opcode::True => w.write_all(b"True"),
            Opcode::UGetCUNop => w.write_all(b"UGetCUNop"),
            Opcode::UnsetG => w.write_all(b"UnsetG"),
            Opcode::UnsetL(id) => {
                w.write_all(b"UnsetL ")?;
                print_local(w, id)
            }
            Opcode::UnsetM(n, mk) => {
                w.write_all(b"UnsetM ")?;
                print_stack_index(w, n)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::Vec(id) => {
                w.write_all(b"Vec ")?;
                print_adata_id(w, id)
            }
            Opcode::VerifyOutType(id) => {
                w.write_all(b"VerifyOutType ")?;
                print_param_id(w, id)
            }
            Opcode::VerifyParamType(id) => {
                w.write_all(b"VerifyParamType ")?;
                print_param_id(w, id)
            }
            Opcode::VerifyParamTypeTS(id) => {
                w.write_all(b"VerifyParamTypeTS ")?;
                print_param_id(w, id)
            }
            Opcode::VerifyRetNonNullC => w.write_all(b"VerifyRetNonNullC"),
            Opcode::VerifyRetTypeC => w.write_all(b"VerifyRetTypeC"),
            Opcode::VerifyRetTypeTS => w.write_all(b"VerifyRetTypeTS"),
            Opcode::WHResult => w.write_all(b"WHResult"),
            Opcode::Yield => w.write_all(b"Yield"),
            Opcode::YieldK => w.write_all(b"YieldK"),
        }
    }
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    print::{print_fcall_args, print_int, print_label},
    write::{angle, concat_by, concat_str, concat_str_by, quotes, Error},
};
use ffi::{Maybe, Pair, Str};
use hash::HashSet;
use hhbc_ast::{
    AdataId, BareThisOp, ClassId, CollectionType, ConstId, ContCheckOp, FatalOp, FunctionId,
    IncDecOp, InitPropOp, IsLogAsDynamicCallOp, IsTypeOp, IterArgs, MOpMode, MemberKey, MethodId,
    OODeclExistsOp, ObjMethodOp, Opcode, ParamId, PropId, QueryMOp, ReadonlyOp, SetOpOp,
    SetRangeOp, SilenceOp, SpecialClsRef, StackIndex, SwitchKind, TypeStructResolveOp,
};
use hhbc_string_utils::float;
use iterator::IterId;
use label::Label;
use local::Local;
use print_opcode::PrintOpcodeTypes;
use std::io::{Result, Write};
use std::write;
use write_bytes::write_bytes;

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
}

fn print_adata_id(w: &mut dyn Write, id: &AdataId<'_>) -> Result<()> {
    write_bytes!(w, "@{}", id)
}

fn print_class_id(w: &mut dyn Write, id: &ClassId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_const_id(w: &mut dyn Write, id: &ConstId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_collection_type(w: &mut dyn Write, ct: &CollectionType) -> Result<()> {
    use CollectionType as CT;
    match *ct {
        CT::Vector => w.write_all(b"Vector"),
        CT::Map => w.write_all(b"Map"),
        CT::Set => w.write_all(b"Set"),
        CT::Pair => w.write_all(b"Pair"),
        CT::ImmVector => w.write_all(b"ImmVector"),
        CT::ImmMap => w.write_all(b"ImmMap"),
        CT::ImmSet => w.write_all(b"ImmSet"),
        _ => panic!("Enum value does not match one of listed variants"),
    }
}

fn print_eq_op(w: &mut dyn Write, op: &SetOpOp) -> Result<()> {
    w.write_all(match *op {
        SetOpOp::PlusEqual => b"PlusEqual",
        SetOpOp::MinusEqual => b"MinusEqual",
        SetOpOp::MulEqual => b"MulEqual",
        SetOpOp::ConcatEqual => b"ConcatEqual",
        SetOpOp::DivEqual => b"DivEqual",
        SetOpOp::PowEqual => b"PowEqual",
        SetOpOp::ModEqual => b"ModEqual",
        SetOpOp::AndEqual => b"AndEqual",
        SetOpOp::OrEqual => b"OrEqual",
        SetOpOp::XorEqual => b"XorEqual",
        SetOpOp::SlEqual => b"SlEqual",
        SetOpOp::SrEqual => b"SrEqual",
        SetOpOp::PlusEqualO => b"PlusEqualO",
        SetOpOp::MinusEqualO => b"MinusEqualO",
        SetOpOp::MulEqualO => b"MulEqualO",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_fatal_op(w: &mut dyn Write, f: &FatalOp) -> Result<()> {
    match *f {
        FatalOp::Parse => w.write_all(b"Fatal Parse"),
        FatalOp::Runtime => w.write_all(b"Fatal Runtime"),
        FatalOp::RuntimeOmitFrame => w.write_all(b"Fatal RuntimeOmitFrame"),
        _ => panic!("Enum value does not match one of listed variants"),
    }
}

fn print_function_id(w: &mut dyn Write, id: &FunctionId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_istype_op(w: &mut dyn Write, op: &IsTypeOp) -> Result<()> {
    use IsTypeOp as Op;
    match *op {
        Op::Null => w.write_all(b"Null"),
        Op::Bool => w.write_all(b"Bool"),
        Op::Int => w.write_all(b"Int"),
        Op::Dbl => w.write_all(b"Dbl"),
        Op::Str => w.write_all(b"Str"),
        Op::Obj => w.write_all(b"Obj"),
        Op::Res => w.write_all(b"Res"),
        Op::Scalar => w.write_all(b"Scalar"),
        Op::Keyset => w.write_all(b"Keyset"),
        Op::Dict => w.write_all(b"Dict"),
        Op::Vec => w.write_all(b"Vec"),
        Op::ArrLike => w.write_all(b"ArrLike"),
        Op::LegacyArrLike => w.write_all(b"LegacyArrLike"),
        Op::ClsMeth => w.write_all(b"ClsMeth"),
        Op::Func => w.write_all(b"Func"),
        Op::Class => w.write_all(b"Class"),
        _ => panic!("Enum value does not match one of listed variants"),
    }
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

fn print_incdec_op(w: &mut dyn Write, op: &IncDecOp) -> Result<()> {
    w.write_all(match *op {
        IncDecOp::PreInc => b"PreInc",
        IncDecOp::PostInc => b"PostInc",
        IncDecOp::PreDec => b"PreDec",
        IncDecOp::PostDec => b"PostDec",
        IncDecOp::PreIncO => b"PreIncO",
        IncDecOp::PostIncO => b"PostIncO",
        IncDecOp::PreDecO => b"PreDecO",
        IncDecOp::PostDecO => b"PostDecO",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_local(w: &mut dyn Write, local: &Local<'_>) -> Result<()> {
    match local {
        Local::Unnamed(id) => {
            w.write_all(b"_")?;
            print_int(w, id)
        }
        Local::Named(id) => w.write_all(id),
    }
}

fn print_member_opmode(w: &mut dyn Write, m: &MOpMode) -> Result<()> {
    use MOpMode as M;
    w.write_all(match *m {
        M::None => b"None",
        M::Warn => b"Warn",
        M::Define => b"Define",
        M::Unset => b"Unset",
        M::InOut => b"InOut",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_method_id(w: &mut dyn Write, id: &MethodId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
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
            quotes(w, |w| w.write_all(&escaper::escape_bstr(s.as_bstr())))?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::EI(i, op) => {
            concat_str(w, ["EI:", i.to_string().as_ref()])?;
            w.write_all(b" ")?;
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

fn print_null_flavor(w: &mut dyn Write, f: &ObjMethodOp) -> Result<()> {
    w.write_all(match *f {
        ObjMethodOp::NullThrows => b"NullThrows",
        ObjMethodOp::NullSafe => b"NullSafe",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_param_id(w: &mut dyn Write, param_id: &ParamId<'_>) -> Result<()> {
    match param_id {
        ParamId::ParamUnnamed(i) => w.write_all(i.to_string().as_bytes()),
        ParamId::ParamNamed(s) => w.write_all(s),
    }
}

fn print_prop_id(w: &mut dyn Write, id: &PropId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_query_op(w: &mut dyn Write, q: QueryMOp) -> Result<()> {
    w.write_all(match q {
        QueryMOp::CGet => b"CGet",
        QueryMOp::CGetQuiet => b"CGetQuiet",
        QueryMOp::Isset => b"Isset",
        QueryMOp::InOut => b"InOut",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_readonly_op(w: &mut dyn Write, op: &ReadonlyOp) -> Result<()> {
    w.write_all(match *op {
        ReadonlyOp::Readonly => b"Readonly",
        ReadonlyOp::Mutable => b"Mutable",
        ReadonlyOp::Any => b"Any",
        ReadonlyOp::CheckROCOW => b"CheckROCOW",
        ReadonlyOp::CheckMutROCOW => b"CheckMutROCOW",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_special_cls_ref(w: &mut dyn Write, cls_ref: &SpecialClsRef) -> Result<()> {
    w.write_all(match *cls_ref {
        SpecialClsRef::LateBoundCls => b"LateBoundCls",
        SpecialClsRef::SelfCls => b"SelfCls",
        SpecialClsRef::ParentCls => b"ParentCls",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_stack_index(w: &mut dyn Write, si: &StackIndex) -> Result<()> {
    w.write_all(si.to_string().as_bytes())
}

fn print_sswitch(
    w: &mut dyn Write,
    cases: &[Str<'_>],
    targets: &[Label],
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    match (cases, targets) {
        ([], _) | (_, []) => Err(Error::fail("sswitch should have at least one case").into()),
        ([rest_cases @ .., _last_case], [rest_targets @ .., last_target]) => {
            w.write_all(b"SSwitch ")?;
            let rest: Vec<_> = rest_cases
                .iter()
                .zip(rest_targets)
                .map(|(case, target)| Pair(case, target))
                .collect();
            angle(w, |w| {
                concat_by(w, " ", rest, |w, Pair(case, target)| {
                    write_bytes!(w, r#""{}":"#, escaper::escape_bstr(case.as_bstr()))?;
                    print_label(w, target, dv_labels)
                })?;
                w.write_all(b" -:")?;
                print_label(w, last_target, dv_labels)
            })
        }
    }
}

fn print_switch(
    w: &mut dyn Write,
    kind: &SwitchKind,
    base: &i64,
    labels: &[Label],
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    w.write_all(b"Switch ")?;
    w.write_all(match *kind {
        SwitchKind::Bounded => b"Bounded ",
        SwitchKind::Unbounded => b"Unbounded ",
        _ => panic!("Enum value does not match one of listed variants"),
    })?;
    w.write_all(base.to_string().as_bytes())?;
    w.write_all(b" ")?;
    angle(w, |w| {
        concat_by(w, " ", labels, |w, label| print_label(w, label, dv_labels))
    })
}

fn print_shape_fields(w: &mut dyn Write, sf: &[&str]) -> Result<()> {
    concat_by(w, " ", sf, |w, f| {
        quotes(w, |w| w.write_all(escaper::escape(*f).as_bytes()))
    })
}

impl<'a, 'b, 'c> PrintOpcodeTypes for PrintOpcode<'a, 'b, 'c> {
    type Write = dyn std::io::Write + 'c;
    type Error = std::io::Error;
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
                w.write_all(s)
            }
            Opcode::AssertRATStk(n, s) => {
                write_bytes!(w, "AssertRATStk {} {}", n, s,)
            }
            Opcode::Await => w.write_all(b"Await"),
            Opcode::AwaitAll(range) => {
                write!(w, "AwaitAll L:{}+{}", range.start, range.len)
            }
            Opcode::BareThis(op) => concat_str_by(
                w,
                " ",
                [
                    "BareThis",
                    match *op {
                        BareThisOp::Notice => "Notice",
                        BareThisOp::NoNotice => "NoNotice",
                        BareThisOp::NeverNull => "NeverNull",
                        _ => panic!("Enum value does not match one of listed variants"),
                    },
                ],
            ),
            Opcode::BaseC(si, m) => {
                w.write_all(b"BaseC ")?;
                print_stack_index(w, si)?;
                w.write_all(b" ")?;
                print_member_opmode(w, m)
            }
            Opcode::BaseGC(si, m) => {
                w.write_all(b"BaseGC ")?;
                print_stack_index(w, si)?;
                w.write_all(b" ")?;
                print_member_opmode(w, m)
            }
            Opcode::BaseGL(id, m) => {
                w.write_all(b"BaseGL ")?;
                print_local(w, id)?;
                w.write_all(b" ")?;
                print_member_opmode(w, m)
            }
            Opcode::BaseH => w.write_all(b"BaseH"),
            Opcode::BaseL(id, m, op) => {
                w.write_all(b"BaseL ")?;
                print_local(w, id)?;
                w.write_all(b" ")?;
                print_member_opmode(w, m)?;
                w.write_all(b" ")?;
                print_readonly_op(w, op)
            }
            Opcode::BaseSC(si1, si2, m, op) => {
                w.write_all(b"BaseSC ")?;
                print_stack_index(w, si1)?;
                w.write_all(b" ")?;
                print_stack_index(w, si2)?;
                w.write_all(b" ")?;
                print_member_opmode(w, m)?;
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
            Opcode::CombineAndResolveTypeStruct(n) => concat_str_by(
                w,
                " ",
                ["CombineAndResolveTypeStruct", n.to_string().as_str()],
            ),
            Opcode::Concat => w.write_all(b"Concat"),
            Opcode::ConcatN(n) => concat_str_by(w, " ", ["ConcatN", n.to_string().as_str()]),
            Opcode::ContCheck(ContCheckOp::CheckStarted) => w.write_all(b"ContCheck CheckStarted"),
            Opcode::ContCheck(ContCheckOp::IgnoreStarted) => {
                w.write_all(b"ContCheck IgnoreStarted")
            }
            Opcode::ContCheck(_) => panic!("invalid ContCheck value"),
            Opcode::ContCurrent => w.write_all(b"ContCurrent"),
            Opcode::ContEnter => w.write_all(b"ContEnter"),
            Opcode::ContGetReturn => w.write_all(b"ContGetReturn"),
            Opcode::ContKey => w.write_all(b"ContKey"),
            Opcode::ContRaise => w.write_all(b"ContRaise"),
            Opcode::ContValid => w.write_all(b"ContValid"),
            Opcode::CreateCl(n, cid) => concat_str_by(
                w,
                " ",
                ["CreateCl", n.to_string().as_str(), cid.to_string().as_str()],
            ),
            Opcode::CreateCont => w.write_all(b"CreateCont"),
            Opcode::DblAsBits => todo!(),
            Opcode::Dict(id) => {
                w.write_all(b"Dict ")?;
                print_adata_id(w, id)
            }
            Opcode::Dim(m, mk) => {
                w.write_all(b"Dim ")?;
                print_member_opmode(w, m)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::Dir => w.write_all(b"Dir"),
            Opcode::Div => w.write_all(b"Div"),
            Opcode::Double(d) => write!(w, "Double {}", float::to_string(*d)),
            Opcode::Dup => w.write_all(b"Dup"),
            Opcode::EntryNop => w.write_all(b"EntryNop"),
            Opcode::Eq => w.write_all(b"Eq"),
            Opcode::Eval => w.write_all(b"Eval"),
            Opcode::Exit => w.write_all(b"Exit"),
            Opcode::FCallClsMethod(fcall_args, hint, log) => {
                w.write_all(b"FCallClsMethod ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)?;
                write_bytes!(w, r#" "{}" "#, hint)?;
                w.write_all(match *log {
                    IsLogAsDynamicCallOp::LogAsDynamicCall => b"LogAsDynamicCall",
                    IsLogAsDynamicCallOp::DontLogAsDynamicCall => b"DontLogAsDynamicCall",
                    _ => panic!("Enum value does not match one of listed variants"),
                })
            }
            Opcode::FCallClsMethodD(fcall_args, hint, class, method) => {
                w.write_all(b"FCallClsMethodD ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)?;
                write_bytes!(w, r#" "{}" "#, hint)?;
                print_class_id(w, class)?;
                w.write_all(b" ")?;
                print_method_id(w, method)
            }
            Opcode::FCallClsMethodS(fcall_args, hint, clsref) => {
                w.write_all(b"FCallClsMethodS ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)?;
                write_bytes!(w, r#" "{}" "#, hint)?;
                print_special_cls_ref(w, clsref)
            }
            Opcode::FCallClsMethodSD(fcall_args, hint, clsref, method) => {
                w.write_all(b"FCallClsMethodSD ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)?;
                write_bytes!(w, r#" "{}" "#, hint)?;
                print_special_cls_ref(w, clsref)?;
                w.write_all(b" ")?;
                print_method_id(w, method)
            }
            Opcode::FCallCtor(fcall_args, hint) => {
                w.write_all(b"FCallCtor ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)?;
                write_bytes!(w, r#" "{}""#, hint)
            }
            Opcode::FCallFunc(fcall_args) => {
                w.write_all(b"FCallFunc ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)
            }
            Opcode::FCallFuncD(fcall_args, func) => {
                w.write_all(b"FCallFuncD ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)?;
                w.write_all(b" ")?;
                print_function_id(w, func)
            }
            Opcode::FCallObjMethod(fcall_args, hint, flavor) => {
                w.write_all(b"FCallObjMethod ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)?;
                write_bytes!(w, r#" "{}" "#, hint)?;
                print_null_flavor(w, flavor)
            }
            Opcode::FCallObjMethodD(fcall_args, hint, flavor, method) => {
                w.write_all(b"FCallObjMethodD ")?;
                print_fcall_args(w, fcall_args, self.dv_labels)?;
                write_bytes!(w, r#" "{}" "#, hint)?;
                print_null_flavor(w, flavor)?;
                w.write_all(b" ")?;
                print_method_id(w, method)
            }
            Opcode::False => w.write_all(b"False"),
            Opcode::Fatal(fatal_op) => print_fatal_op(w, fatal_op),
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
                print_incdec_op(w, op)
            }
            Opcode::IncDecL(id, op) => {
                w.write_all(b"IncDecL ")?;
                print_local(w, id)?;
                w.write_all(b" ")?;
                print_incdec_op(w, op)
            }
            Opcode::IncDecM(i, op, mk) => {
                w.write_all(b"IncDecM ")?;
                print_int(w, i)?;
                w.write_all(b" ")?;
                print_incdec_op(w, op)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::IncDecS(op) => {
                w.write_all(b"IncDecS ")?;
                print_incdec_op(w, op)
            }
            Opcode::Incl => w.write_all(b"Incl"),
            Opcode::InclOnce => w.write_all(b"InclOnce"),
            Opcode::InitProp(id, op) => {
                w.write_all(b"InitProp ")?;
                print_prop_id(w, id)?;
                w.write_all(b" ")?;
                match *op {
                    InitPropOp::Static => w.write_all(b"Static"),
                    InitPropOp::NonStatic => w.write_all(b"NonStatic"),
                    _ => panic!("Enum value does not match one of listed variants"),
                }
            }
            Opcode::InstanceOf => w.write_all(b"InstanceOf"),
            Opcode::InstanceOfD(id) => {
                w.write_all(b"InstanceOfD ")?;
                print_class_id(w, id)
            }
            Opcode::Int(i) => concat_str_by(w, " ", ["Int", i.to_string().as_str()]),
            Opcode::IsLateBoundCls => w.write_all(b"IsLateBoundCls"),
            Opcode::IsTypeC(op) => {
                w.write_all(b"IsTypeC ")?;
                print_istype_op(w, op)
            }
            Opcode::IsTypeL(local, op) => {
                w.write_all(b"IsTypeL ")?;
                print_local(w, local)?;
                w.write_all(b" ")?;
                print_istype_op(w, op)
            }
            Opcode::IsTypeStructC(op) => concat_str_by(
                w,
                " ",
                [
                    "IsTypeStructC",
                    match *op {
                        TypeStructResolveOp::Resolve => "Resolve",
                        TypeStructResolveOp::DontResolve => "DontResolve",
                        _ => panic!("Enum value does not match one of listed variants"),
                    },
                ],
            ),
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
                print_label(w, label, self.dv_labels)
            }
            Opcode::IterNext(iter_args, label) => {
                w.write_all(b"IterNext ")?;
                print_iter_args(w, iter_args)?;
                w.write_all(b" ")?;
                print_label(w, label, self.dv_labels)
            }
            Opcode::Jmp(l) => {
                w.write_all(b"Jmp ")?;
                print_label(w, l, self.dv_labels)
            }
            Opcode::JmpNS(l) => {
                w.write_all(b"JmpNS ")?;
                print_label(w, l, self.dv_labels)
            }
            Opcode::JmpNZ(l) => {
                w.write_all(b"JmpNZ ")?;
                print_label(w, l, self.dv_labels)
            }
            Opcode::JmpZ(l) => {
                w.write_all(b"JmpZ ")?;
                print_label(w, l, self.dv_labels)
            }
            Opcode::Keyset(id) => {
                w.write_all(b"Keyset ")?;
                print_adata_id(w, id)
            }
            Opcode::LIterFree(..) => todo!(),
            Opcode::LIterInit(..) => todo!(),
            Opcode::LIterNext(..) => todo!(),
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
                print_label(w, label, self.dv_labels)?;
                write!(w, " L:{}+{}", range.start, range.len)
            }
            Opcode::MemoGetEager([label1, label2], range) => {
                w.write_all(b"MemoGetEager ")?;
                print_label(w, label1, self.dv_labels)?;
                w.write_all(b" ")?;
                print_label(w, label2, self.dv_labels)?;
                write!(w, " L:{}+{}", range.start, range.len)
            }
            Opcode::MemoSet(range) => {
                write!(w, "MemoSet L:{}+{}", range.start, range.len)
            }
            Opcode::MemoSetEager(range) => {
                write!(w, "MemoSetEager L:{}+{}", range.start, range.len)
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
                concat_str_by(w, " ", ["NewDictArray", i.to_string().as_str()])
            }
            Opcode::NewKeysetArray(i) => {
                concat_str_by(w, " ", ["NewKeysetArray", i.to_string().as_str()])
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
                let ls: Vec<&str> = l.as_ref().iter().map(|s| s.unsafe_as_str()).collect();
                w.write_all(b"NewStructDict ")?;
                angle(w, |w| print_shape_fields(w, &ls[0..]))
            }
            Opcode::NewVec(i) => concat_str_by(w, " ", ["NewVec", i.to_string().as_str()]),
            Opcode::Nop => w.write_all(b"Nop"),
            Opcode::Not => w.write_all(b"Not"),
            Opcode::Null => w.write_all(b"Null"),
            Opcode::NullUninit => w.write_all(b"NullUninit"),
            Opcode::OODeclExists(k) => concat_str_by(
                w,
                " ",
                [
                    "OODeclExists",
                    match *k {
                        OODeclExistsOp::Class => "Class",
                        OODeclExistsOp::Interface => "Interface",
                        OODeclExistsOp::Trait => "Trait",
                        _ => panic!("Enum value does not match one of listed variants"),
                    },
                ],
            ),
            Opcode::ParentCls => w.write_all(b"ParentCls"),
            Opcode::PopC => w.write_all(b"PopC"),
            Opcode::PopL(id) => {
                w.write_all(b"PopL ")?;
                print_local(w, id)
            }
            Opcode::PopU => w.write_all(b"PopU"),
            Opcode::PopU2 => todo!(),
            Opcode::Pow => w.write_all(b"Pow"),
            Opcode::Print => w.write_all(b"Print"),
            Opcode::PushL(id) => {
                w.write_all(b"PushL ")?;
                print_local(w, id)
            }
            Opcode::QueryM(n, op, mk) => {
                w.write_all(b"QueryM ")?;
                print_int(w, n)?;
                w.write_all(b" ")?;
                print_query_op(w, *op)?;
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
            Opcode::RetM(p) => concat_str_by(w, " ", ["RetM", p.to_string().as_str()]),
            Opcode::SSwitch { cases, targets } => {
                print_sswitch(w, cases.as_ref(), targets.as_ref(), self.dv_labels)
            }
            Opcode::Same => w.write_all(b"Same"),
            Opcode::Select => todo!(),
            Opcode::SelfCls => w.write_all(b"SelfCls"),
            Opcode::SetG => w.write_all(b"SetG"),
            Opcode::SetImplicitContextByValue => w.write_all(b"SetImplicitContextByValue"),
            Opcode::SetL(local) => {
                w.write_all(b"SetL ")?;
                print_local(w, local)
            }
            Opcode::SetM(i, mk) => {
                w.write_all(b"SetM ")?;
                print_int(w, i)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::SetOpG(op) => {
                w.write_all(b"SetOpG ")?;
                print_eq_op(w, op)
            }
            Opcode::SetOpL(id, op) => {
                w.write_all(b"SetOpL ")?;
                print_local(w, id)?;
                w.write_all(b" ")?;
                print_eq_op(w, op)
            }
            Opcode::SetOpM(i, op, mk) => {
                w.write_all(b"SetOpM ")?;
                print_int(w, i)?;
                w.write_all(b" ")?;
                print_eq_op(w, op)?;
                w.write_all(b" ")?;
                print_member_key(w, mk)
            }
            Opcode::SetOpS(op) => {
                w.write_all(b"SetOpS ")?;
                print_eq_op(w, op)
            }
            Opcode::SetRangeM(i, s, op) => {
                w.write_all(b"SetRangeM ")?;
                print_int(w, i)?;
                w.write_all(b" ")?;
                print_int(w, &(*s as usize))?;
                w.write_all(b" ")?;
                w.write_all(match *op {
                    SetRangeOp::Forward => b"Forward",
                    SetRangeOp::Reverse => b"Reverse",
                    _ => panic!("Enum value does not match one of listed variants"),
                })
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
                match *op {
                    SilenceOp::Start => w.write_all(b"Start"),
                    SilenceOp::End => w.write_all(b"End"),
                    _ => panic!("Enum value does not match one of listed variants"),
                }
            }
            Opcode::String(s) => {
                w.write_all(b"String ")?;
                quotes(w, |w| w.write_all(&escaper::escape_bstr(s.as_bstr())))
            }
            Opcode::Sub => w.write_all(b"Sub"),
            Opcode::SubO => w.write_all(b"SubO"),
            Opcode::Switch(kind, base, targets) => {
                print_switch(w, kind, base, targets.as_ref(), self.dv_labels)
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
                print_int(w, n)?;
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
            Opcode::VerifyRetNonNullC => todo!(),
            Opcode::VerifyRetTypeC => w.write_all(b"VerifyRetTypeC"),
            Opcode::VerifyRetTypeTS => w.write_all(b"VerifyRetTypeTS"),
            Opcode::WHResult => w.write_all(b"WHResult"),
            Opcode::Yield => w.write_all(b"Yield"),
            Opcode::YieldK => w.write_all(b"YieldK"),
        }
    }
}

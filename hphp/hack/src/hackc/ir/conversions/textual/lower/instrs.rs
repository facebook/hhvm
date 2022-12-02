// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::func_builder::TransformInstr;
use ir::instr::CallDetail;
use ir::instr::CmpOp;
use ir::instr::HasLoc;
use ir::instr::HasOperands;
use ir::instr::Hhbc;
use ir::instr::Terminator;
use ir::BareThisOp;
use ir::Call;
use ir::Constant;
use ir::FCallArgsFlags;
use ir::Func;
use ir::FuncBuilder;
use ir::FuncBuilderEx as _;
use ir::Instr;
use ir::InstrId;
use ir::IsTypeOp;
use ir::LocId;
use ir::LocalId;
use ir::MethodId;
use ir::ObjMethodOp;
use ir::SpecialClsRef;
use ir::UnitBytesId;
use ir::ValueId;

use super::func_builder::FuncBuilderEx as _;
use crate::class::IsStatic;
use crate::func::MethodInfo;
use crate::hack;

/// Lower individual Instrs in the Func to simpler forms.
pub(crate) fn lower_instrs(
    builder: &mut FuncBuilder<'_>,
    method_info: Option<Arc<MethodInfo<'_>>>,
) {
    let mut lowerer = LowerInstrs {
        changed: false,
        method_info,
    };

    let mut bid = Func::ENTRY_BID;
    while bid.0 < builder.func.blocks.len() as u32 {
        lowerer.changed = true;
        while lowerer.changed {
            // The lowered instructions may have emitted stuff that needs to be
            // lowered again. This could be optimized by only setting `changed`
            // when we think we need to loop.
            lowerer.changed = false;
            builder.rewrite_block(bid, &mut lowerer);
        }
        bid.0 += 1;
    }
}

struct LowerInstrs<'a> {
    changed: bool,
    method_info: Option<Arc<MethodInfo<'a>>>,
}

impl LowerInstrs<'_> {
    fn handle_hhbc_with_builtin(&self, hhbc: &Hhbc) -> Option<hack::Hhbc> {
        let builtin = match hhbc {
            Hhbc::Add(..) => hack::Hhbc::Add,
            Hhbc::CastVec(..) => hack::Hhbc::CastVec,
            Hhbc::ClassGetC(..) => hack::Hhbc::ClassGetC,
            Hhbc::ClassHasReifiedGenerics(..) => hack::Hhbc::ClassHasReifiedGenerics,
            Hhbc::CheckClsRGSoft(..) => hack::Hhbc::CheckClsRGSoft,
            Hhbc::CmpOp(_, CmpOp::Eq, _) => hack::Hhbc::CmpEq,
            Hhbc::CmpOp(_, CmpOp::Gt, _) => hack::Hhbc::CmpGt,
            Hhbc::CmpOp(_, CmpOp::Gte, _) => hack::Hhbc::CmpGte,
            Hhbc::CmpOp(_, CmpOp::Lt, _) => hack::Hhbc::CmpLt,
            Hhbc::CmpOp(_, CmpOp::Lte, _) => hack::Hhbc::CmpLte,
            Hhbc::CmpOp(_, CmpOp::NSame, _) => hack::Hhbc::CmpNSame,
            Hhbc::CmpOp(_, CmpOp::Neq, _) => hack::Hhbc::CmpNeq,
            Hhbc::CmpOp(_, CmpOp::Same, _) => hack::Hhbc::CmpSame,
            Hhbc::Concat(..) => hack::Hhbc::Concat,
            Hhbc::Div(..) => hack::Hhbc::Div,
            Hhbc::GetClsRGProp(..) => hack::Hhbc::GetClsRGProp,
            Hhbc::HasReifiedParent(..) => hack::Hhbc::HasReifiedParent,
            Hhbc::IsTypeC(_, IsTypeOp::ArrLike, _) => hack::Hhbc::IsTypeArrLike,
            Hhbc::IsTypeC(_, IsTypeOp::Bool, _) => hack::Hhbc::IsTypeBool,
            Hhbc::IsTypeC(_, IsTypeOp::Class, _) => hack::Hhbc::IsTypeClass,
            Hhbc::IsTypeC(_, IsTypeOp::ClsMeth, _) => hack::Hhbc::IsTypeClsMeth,
            Hhbc::IsTypeC(_, IsTypeOp::Dbl, _) => hack::Hhbc::IsTypeDbl,
            Hhbc::IsTypeC(_, IsTypeOp::Dict, _) => hack::Hhbc::IsTypeDict,
            Hhbc::IsTypeC(_, IsTypeOp::Func, _) => hack::Hhbc::IsTypeFunc,
            Hhbc::IsTypeC(_, IsTypeOp::Int, _) => hack::Hhbc::IsTypeInt,
            Hhbc::IsTypeC(_, IsTypeOp::Keyset, _) => hack::Hhbc::IsTypeKeyset,
            Hhbc::IsTypeC(_, IsTypeOp::LegacyArrLike, _) => hack::Hhbc::IsTypeLegacyArrLike,
            Hhbc::IsTypeC(_, IsTypeOp::Null, _) => hack::Hhbc::IsTypeNull,
            Hhbc::IsTypeC(_, IsTypeOp::Obj, _) => hack::Hhbc::IsTypeObj,
            Hhbc::IsTypeC(_, IsTypeOp::Res, _) => hack::Hhbc::IsTypeRes,
            Hhbc::IsTypeC(_, IsTypeOp::Scalar, _) => hack::Hhbc::IsTypeScalar,
            Hhbc::IsTypeC(_, IsTypeOp::Str, _) => hack::Hhbc::IsTypeStr,
            Hhbc::IsTypeC(_, IsTypeOp::Vec, _) => hack::Hhbc::IsTypeVec,
            Hhbc::Modulo(..) => hack::Hhbc::Modulo,
            Hhbc::Mul(..) => hack::Hhbc::Mul,
            Hhbc::NewDictArray(..) => hack::Hhbc::NewDictArray,
            Hhbc::NewVec(..) => hack::Hhbc::NewVec,
            Hhbc::Not(..) => hack::Hhbc::Not,
            Hhbc::Print(..) => hack::Hhbc::Print,
            Hhbc::Sub(..) => hack::Hhbc::Sub,
            _ => return None,
        };
        Some(builtin)
    }

    fn handle_with_builtin(&self, builder: &mut FuncBuilder<'_>, instr: &Instr) -> Option<Instr> {
        let builtin = match instr {
            Instr::Hhbc(hhbc) => {
                if let Some(hhbc) = self.handle_hhbc_with_builtin(hhbc) {
                    hack::Builtin::Hhbc(hhbc)
                } else {
                    return None;
                }
            }
            _ => return None,
        };
        Some(builder.hack_builtin(builtin, instr.operands(), instr.loc_id()))
    }

    fn verify_out_type(
        &self,
        builder: &mut FuncBuilder<'_>,
        obj: ValueId,
        lid: LocalId,
        loc: LocId,
    ) -> Instr {
        let param = builder
            .func
            .get_param_by_lid(lid)
            .expect("Unknown parameter in verify_out_type()");
        let param_type = param.ty.enforced.clone();
        let pred = builder.emit_is(obj, &param_type, loc);
        builder.emit_hack_builtin(hack::Builtin::VerifyTypePred, &[obj, pred], loc);
        Instr::copy(obj)
    }

    fn verify_param_type_ts(
        &self,
        builder: &mut FuncBuilder<'_>,
        lid: LocalId,
        ts: ValueId,
        loc: LocId,
    ) -> Instr {
        let obj = builder.emit(Instr::Hhbc(Hhbc::CGetL(lid, loc)));
        let builtin = hack::Hhbc::VerifyParamTypeTS;
        builder.emit_hhbc_builtin(builtin, &[obj, ts], loc);
        Instr::tombstone()
    }

    fn verify_ret_type_c(&self, builder: &mut FuncBuilder<'_>, obj: ValueId, loc: LocId) -> Instr {
        let return_type = builder.func.return_type.enforced.clone();
        let pred = builder.emit_is(obj, &return_type, loc);
        builder.emit_hack_builtin(hack::Builtin::VerifyTypePred, &[obj, pred], loc);
        Instr::copy(obj)
    }

    fn emit_special_cls_ref(
        &mut self,
        builder: &mut FuncBuilder<'_>,
        clsref: SpecialClsRef,
        loc: LocId,
    ) -> ValueId {
        match clsref {
            SpecialClsRef::SelfCls => builder.emit(Instr::Hhbc(Hhbc::SelfCls(loc))),
            SpecialClsRef::LateBoundCls => builder.emit(Instr::Hhbc(Hhbc::LateBoundCls(loc))),
            SpecialClsRef::ParentCls => builder.emit(Instr::Hhbc(Hhbc::ParentCls(loc))),
            _ => unreachable!(),
        }
    }
}

impl TransformInstr for LowerInstrs<'_> {
    fn apply(&mut self, _iid: InstrId, instr: Instr, builder: &mut FuncBuilder<'_>) -> Instr {
        use hack::Builtin;

        if let Some(instr) = self.handle_with_builtin(builder, &instr) {
            self.changed = true;
            return instr;
        }

        let instr = match instr {
            // NOTE: Be careful trying to lower most Instr::Call - the details
            // matter and it's hard to rewrite them to preserve those details
            // properly.
            Instr::Call(
                box mut call @ Call {
                    detail: CallDetail::FCallCtor,
                    ..
                },
            ) => {
                let flavor = ObjMethodOp::NullThrows;
                let method = MethodId::from_str("__construct", &builder.strings);
                call.detail = CallDetail::FCallObjMethodD { flavor, method };
                Instr::Call(Box::new(call))
            }
            Instr::Hhbc(Hhbc::BareThis(_op, loc)) => {
                let this = builder.strings.intern_str("$this");
                let lid = LocalId::Named(this);
                Instr::Hhbc(Hhbc::CGetL(lid, loc))
            }
            Instr::Hhbc(Hhbc::CheckThis(loc)) => {
                let builtin = hack::Hhbc::CheckThis;
                let op = BareThisOp::NoNotice;
                let this = builder.emit(Instr::Hhbc(Hhbc::BareThis(op, loc)));
                builder.hhbc_builtin(builtin, &[this], loc)
            }
            Instr::Hhbc(Hhbc::InstanceOfD(vid, cid, loc)) => {
                let cid = builder.emit_constant(Constant::String(cid.id));
                builder.hack_builtin(Builtin::IsType, &[vid, cid], loc)
            }
            Instr::Hhbc(Hhbc::LateBoundCls(loc)) => {
                match self.method_info.as_ref().unwrap().is_static {
                    IsStatic::Static => {
                        let this = builder.emit(Instr::Hhbc(Hhbc::This(loc)));
                        builder.hack_builtin(Builtin::GetClass, &[this], loc)
                    }
                    IsStatic::NonStatic => {
                        let this = builder.emit(Instr::Hhbc(Hhbc::This(loc)));
                        builder.hack_builtin(Builtin::GetStaticClass, &[this], loc)
                    }
                }
            }
            Instr::Hhbc(Hhbc::LockObj(obj, loc)) => {
                builder.emit_hhbc_builtin(hack::Hhbc::LockObj, &[obj], loc);
                Instr::copy(obj)
            }
            Instr::Hhbc(Hhbc::NewObj(cls, loc)) => {
                let method = MethodId::from_str("__factory", &builder.strings);
                let operands = vec![cls].into_boxed_slice();
                let context = UnitBytesId::NONE;
                let flavor = ObjMethodOp::NullThrows;
                let detail = CallDetail::FCallObjMethodD { flavor, method };
                let flags = FCallArgsFlags::default();
                Instr::Call(Box::new(Call {
                    operands,
                    context,
                    detail,
                    flags,
                    num_rets: 1,
                    inouts: None,
                    readonly: None,
                    loc,
                }))
            }
            Instr::Hhbc(Hhbc::NewObjD(clsid, loc)) => {
                let cls = builder.emit(Instr::Hhbc(Hhbc::ResolveClass(clsid, loc)));
                let method = MethodId::from_str("__factory", &builder.strings);
                let operands = vec![cls].into_boxed_slice();
                let context = UnitBytesId::NONE;
                let flavor = ObjMethodOp::NullThrows;
                let detail = CallDetail::FCallObjMethodD { flavor, method };
                let flags = FCallArgsFlags::default();
                Instr::Call(Box::new(Call {
                    operands,
                    context,
                    detail,
                    flags,
                    num_rets: 1,
                    inouts: None,
                    readonly: None,
                    loc,
                }))
            }
            Instr::Hhbc(Hhbc::NewObjS(clsref, loc)) => {
                let cls = self.emit_special_cls_ref(builder, clsref, loc);
                Instr::Hhbc(Hhbc::NewObj(cls, loc))
            }
            Instr::Hhbc(Hhbc::VerifyOutType(vid, lid, loc)) => {
                self.verify_out_type(builder, vid, lid, loc)
            }
            Instr::Hhbc(Hhbc::VerifyParamTypeTS(ts, lid, loc)) => {
                self.verify_param_type_ts(builder, lid, ts, loc)
            }
            Instr::Hhbc(Hhbc::VerifyRetTypeC(vid, loc)) => {
                self.verify_ret_type_c(builder, vid, loc)
            }
            Instr::Terminator(Terminator::Exit(ops, loc)) => {
                let builtin = hack::Hhbc::Exit;
                builder.emit_hhbc_builtin(builtin, &[ops], loc);
                Instr::unreachable()
            }
            Instr::Terminator(Terminator::Throw(value, loc)) => {
                let builtin = hack::Hhbc::Throw;
                builder.emit_hhbc_builtin(builtin, &[value], loc);
                Instr::unreachable()
            }
            Instr::Terminator(Terminator::RetM(ops, loc)) => {
                // ret a, b;
                // =>
                // ret vec[a, b];
                let builtin = hack::Hhbc::NewVec;
                let vec = builder.emit_hhbc_builtin(builtin, &ops, loc);
                Instr::ret(vec, loc)
            }
            instr => {
                return instr;
            }
        };
        self.changed = true;
        instr
    }
}

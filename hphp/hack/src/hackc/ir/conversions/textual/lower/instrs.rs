#![allow(unused)]
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::func_builder::TransformInstr;
use ir::instr::CmpOp;
use ir::instr::HasLoc;
use ir::instr::HasOperands;
use ir::instr::Hhbc;
use ir::instr::Terminator;
use ir::Constant;
use ir::Func;
use ir::FuncBuilder;
use ir::FuncBuilderEx as _;
use ir::Instr;
use ir::InstrId;
use ir::IsTypeOp;
use ir::LocId;
use ir::LocalId;
use ir::ValueId;

use super::func_builder::FuncBuilderEx as _;
use crate::func::MethodInfo;
use crate::hack;

/// Lower individual Instrs in the Func to simpler forms.
pub(crate) fn lower_instrs(builder: &mut FuncBuilder<'_>, method_info: Option<&MethodInfo<'_>>) {
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
    method_info: Option<&'a MethodInfo<'a>>,
}

impl LowerInstrs<'_> {
    fn handle_hhbc_with_builtin(&self, hhbc: &Hhbc) -> Option<hack::Hhbc> {
        let builtin = match hhbc {
            Hhbc::Add(..) => hack::Hhbc::Add,
            Hhbc::ClassGetC(..) => hack::Hhbc::ClassGetC,
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
            Hhbc::IsTypeC(_, IsTypeOp::ArrLike, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Bool, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Class, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::ClsMeth, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Dbl, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Dict, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Func, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Int, _) => hack::Hhbc::IsTypeInt,
            Hhbc::IsTypeC(_, IsTypeOp::Keyset, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::LegacyArrLike, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Null, _) => hack::Hhbc::IsTypeNull,
            Hhbc::IsTypeC(_, IsTypeOp::Obj, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Res, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Scalar, _) => todo!(),
            Hhbc::IsTypeC(_, IsTypeOp::Str, _) => hack::Hhbc::IsTypeStr,
            Hhbc::IsTypeC(_, IsTypeOp::Vec, _) => todo!(),
            Hhbc::Modulo(..) => hack::Hhbc::Modulo,
            Hhbc::Mul(..) => hack::Hhbc::Mul,
            Hhbc::NewDictArray(..) => hack::Hhbc::NewDictArray,
            Hhbc::NewObjS(..) => hack::Hhbc::NewObj,
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
        let builtin = hack::Builtin::Hhbc(hack::Hhbc::VerifyParamTypeTS);
        builder.emit_hack_builtin(builtin, &[obj, ts], loc);
        Instr::tombstone()
    }

    fn verify_ret_type_c(&self, builder: &mut FuncBuilder<'_>, obj: ValueId, loc: LocId) -> Instr {
        let return_type = builder.func.return_type.enforced.clone();
        let pred = builder.emit_is(obj, &return_type, loc);
        builder.emit_hack_builtin(hack::Builtin::VerifyTypePred, &[obj, pred], loc);
        Instr::copy(obj)
    }
}

impl TransformInstr for LowerInstrs<'_> {
    fn apply(&mut self, _iid: InstrId, instr: Instr, builder: &mut FuncBuilder<'_>) -> Instr {
        if let Some(instr) = self.handle_with_builtin(builder, &instr) {
            self.changed = true;
            return instr;
        }

        let instr = match instr {
            Instr::Hhbc(Hhbc::BareThis(_op, loc)) => {
                let this = builder.strings.intern_str("$this");
                let lid = LocalId::Named(this);
                Instr::Hhbc(Hhbc::CGetL(lid, loc))
            }
            Instr::Hhbc(Hhbc::CheckThis(loc)) => {
                let builtin = hack::Builtin::Hhbc(hack::Hhbc::CheckThis);
                let op = ir::BareThisOp::NoNotice;
                let this = builder.emit(Instr::Hhbc(Hhbc::BareThis(op, loc)));
                builder.hack_builtin(builtin, &[this], loc)
            }
            Instr::Hhbc(Hhbc::InstanceOfD(vid, cid, loc)) => {
                let cid = builder.emit_constant(Constant::String(cid.id));
                builder.hack_builtin(hack::Builtin::IsType, &[vid, cid], loc)
            }
            Instr::Hhbc(Hhbc::LateBoundCls(loc)) => {
                if self.method_info.unwrap().is_static {
                    let this = builder.emit(Instr::Hhbc(Hhbc::This(loc)));
                    builder.hack_builtin(hack::Builtin::GetClass, &[this], loc)
                } else {
                    let this = builder.emit(Instr::Hhbc(Hhbc::This(loc)));
                    builder.hack_builtin(hack::Builtin::GetStaticClass, &[this], loc)
                }
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
                let builtin = hack::Builtin::Hhbc(hack::Hhbc::Exit);
                builder.emit_hack_builtin(builtin, &[ops], loc);
                Instr::unreachable()
            }
            Instr::Terminator(Terminator::Throw(value, loc)) => {
                let builtin = hack::Builtin::Hhbc(hack::Hhbc::Throw);
                builder.emit_hack_builtin(builtin, &[value], loc);
                Instr::unreachable()
            }
            Instr::Terminator(Terminator::RetM(ops, loc)) => {
                // ret a, b;
                // =>
                // ret vec[a, b];
                let builtin = hack::Builtin::Hhbc(hack::Hhbc::NewVec);
                let vec = builder.emit_hack_builtin(builtin, &ops, loc);
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

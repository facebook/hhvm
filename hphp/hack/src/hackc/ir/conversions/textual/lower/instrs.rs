// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::func_builder::TransformInstr;
use ir::instr::CmpOp;
use ir::instr::HasLoc;
use ir::instr::HasOperands;
use ir::instr::Hhbc;
use ir::instr::IsTypeOp;
use ir::Func;
use ir::FuncBuilder;
use ir::FuncBuilderEx as _;
use ir::Instr;
use ir::InstrId;
use ir::LocId;
use ir::ValueId;

use super::func_builder::FuncBuilderEx as _;
use crate::hack;

/// Lower individual Instrs in the Func to simpler forms.
pub(crate) fn lower_instrs<'a>(builder: &mut FuncBuilder<'a>) {
    let mut lowerer = LowerInstrs { changed: false };

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

#[derive(Default)]
struct LowerInstrs {
    changed: bool,
}

impl LowerInstrs {
    fn handle_with_builtin(&self, builder: &mut FuncBuilder<'_>, instr: &Instr) -> Instr {
        let builtin = match instr {
            Instr::Hhbc(Hhbc::CmpOp(_, CmpOp::Eq, _)) => hack::Builtin::Hhbc(hack::Hhbc::CmpEq),
            Instr::Hhbc(Hhbc::CmpOp(_, CmpOp::Gt, _)) => hack::Builtin::Hhbc(hack::Hhbc::CmpGt),
            Instr::Hhbc(Hhbc::CmpOp(_, CmpOp::Gte, _)) => hack::Builtin::Hhbc(hack::Hhbc::CmpGte),
            Instr::Hhbc(Hhbc::CmpOp(_, CmpOp::Lt, _)) => hack::Builtin::Hhbc(hack::Hhbc::CmpLt),
            Instr::Hhbc(Hhbc::CmpOp(_, CmpOp::Lte, _)) => hack::Builtin::Hhbc(hack::Hhbc::CmpLte),
            Instr::Hhbc(Hhbc::CmpOp(_, CmpOp::NSame, _)) => {
                hack::Builtin::Hhbc(hack::Hhbc::CmpNSame)
            }
            Instr::Hhbc(Hhbc::CmpOp(_, CmpOp::Neq, _)) => hack::Builtin::Hhbc(hack::Hhbc::CmpNeq),
            Instr::Hhbc(Hhbc::CmpOp(_, CmpOp::Same, _)) => hack::Builtin::Hhbc(hack::Hhbc::CmpSame),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::ArrLike, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Bool, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Class, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::ClsMeth, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Dbl, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Dict, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Func, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Int, _)) => {
                hack::Builtin::Hhbc(hack::Hhbc::IsTypeInt)
            }
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Keyset, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::LegacyArrLike, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Null, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Obj, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Res, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Scalar, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Str, _)) => todo!(),
            Instr::Hhbc(Hhbc::IsTypeC(_, IsTypeOp::Vec, _)) => todo!(),
            Instr::Hhbc(Hhbc::Modulo(..)) => hack::Builtin::Hhbc(hack::Hhbc::Modulo),
            Instr::Hhbc(Hhbc::Not(..)) => hack::Builtin::Hhbc(hack::Hhbc::Not),
            Instr::Hhbc(Hhbc::Print(..)) => hack::Builtin::Hhbc(hack::Hhbc::Print),
            _ => unreachable!("Unhandled Instr: {instr:#?}"),
        };
        builder.hack_builtin(builtin, instr.operands(), instr.loc_id())
    }

    fn verify_ret_type_c(&self, builder: &mut FuncBuilder<'_>, vid: ValueId, loc: LocId) -> Instr {
        // if !(<vid> is <ret type>) {
        //   verify_failed();
        // }
        let return_type = builder.func.return_type.enforced.clone();
        let pred = builder.emit_is(vid, &return_type, loc);
        let pred = builder.emit(Instr::Hhbc(Hhbc::Not(pred, loc)));
        builder.if_then(pred, loc, |builder| {
            builder.hack_builtin(hack::Builtin::Hhbc(hack::Hhbc::VerifyFailed), &[], loc)
        })
    }
}

impl TransformInstr for LowerInstrs {
    fn apply<'a>(&mut self, _iid: InstrId, instr: Instr, builder: &mut FuncBuilder<'a>) -> Instr {
        let instr = match instr {
            Instr::Hhbc(
                Hhbc::CmpOp(..)
                | Hhbc::IsTypeC(..)
                | Hhbc::Modulo(..)
                | Hhbc::Not(..)
                | Hhbc::Print(..),
            ) => self.handle_with_builtin(builder, &instr),
            Instr::Hhbc(Hhbc::VerifyRetTypeC(vid, loc)) => {
                self.verify_ret_type_c(builder, vid, loc)
            }
            instr => {
                return instr;
            }
        };
        self.changed = true;
        instr
    }
}

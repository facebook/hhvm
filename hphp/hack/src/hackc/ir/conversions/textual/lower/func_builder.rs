// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Helpers for FuncBuilder emitting Textual specific constructs during lowering.

use std::borrow::Cow;

use ir::FuncBuilder;
use ir::Instr;
use ir::LocId;
use ir::ValueId;

use crate::hack;

pub(crate) trait FuncBuilderEx {
    /// Build a hack::Builtin call.
    fn hack_builtin(&mut self, builtin: hack::Builtin, args: &[ValueId], loc: LocId) -> Instr;

    fn emit_hack_builtin(
        &mut self,
        builtin: hack::Builtin,
        args: &[ValueId],
        loc: LocId,
    ) -> ValueId;

    fn hhbc_builtin(&mut self, builtin: hack::Hhbc, args: &[ValueId], loc: LocId) -> Instr {
        self.hack_builtin(hack::Builtin::Hhbc(builtin), args, loc)
    }

    fn emit_hhbc_builtin(&mut self, builtin: hack::Hhbc, args: &[ValueId], loc: LocId) -> ValueId {
        self.emit_hack_builtin(hack::Builtin::Hhbc(builtin), args, loc)
    }
}

impl FuncBuilderEx for FuncBuilder {
    fn hack_builtin(&mut self, builtin: hack::Builtin, args: &[ValueId], loc: LocId) -> Instr {
        let target = Cow::Borrowed(builtin.as_str());
        let values = args.to_vec().into_boxed_slice();
        Instr::Special(ir::instr::Special::Textual(
            ir::instr::Textual::HackBuiltin {
                target,
                values,
                loc,
            },
        ))
    }

    fn emit_hack_builtin(
        &mut self,
        builtin: hack::Builtin,
        args: &[ValueId],
        loc: LocId,
    ) -> ValueId {
        let instr = self.hack_builtin(builtin, args, loc);
        self.emit(instr)
    }
}

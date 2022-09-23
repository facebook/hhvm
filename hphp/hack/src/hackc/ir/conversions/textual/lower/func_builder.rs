// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Helpers for FuncBuilder emitting Textual specific constructs during lowering.

use ir::FuncBuilder;
use ir::Instr;
use ir::LocId;
use ir::ValueId;
use itertools::Itertools;

use crate::hack;

pub(crate) trait FuncBuilderEx {
    /// Build a hack::Builtin call.
    fn hack_builtin(&mut self, _builtin: hack::Builtin, _args: &[ValueId], _loc: LocId) -> Instr;

    fn emit_hack_builtin(
        &mut self,
        builtin: hack::Builtin,
        args: &[ValueId],
        loc: LocId,
    ) -> ValueId;
}

impl<'a> FuncBuilderEx for FuncBuilder<'a> {
    fn hack_builtin(&mut self, builtin: hack::Builtin, args: &[ValueId], loc: LocId) -> Instr {
        use ir::instr::TextualHackBuiltinParam;
        let target = builtin.into_str();
        let params = args
            .iter()
            .map(|_| TextualHackBuiltinParam::Value)
            .collect_vec()
            .into_boxed_slice();
        let values = args.to_vec().into_boxed_slice();
        Instr::Special(ir::instr::Special::Textual(
            ir::instr::Textual::HackBuiltin {
                target,
                params,
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

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::instr;
use crate::BaseType;
use crate::EnforceableType;
use crate::FuncBuilder;
use crate::Instr;
use crate::LocId;
use crate::TypeConstraintFlags;
use crate::ValueId;

/// Helpers for emitting more complex constructs.
pub trait FuncBuilderEx {
    /// Build an if/then:
    ///     jmp_t true_br, false_br
    ///     true_br:
    ///         f()
    ///         jmp false_br
    ///     false_br:
    ///
    /// Note that although the callback only returns a single Instr it can emit
    /// as many instrs as it wants simply returning the final one. In addition
    /// it can return a tombstone to indicate that it already emitted everything
    /// it needed.
    fn if_then(&mut self, pred: ValueId, loc: LocId, f: impl FnOnce(&mut Self) -> Instr) -> Instr;

    /// Emit an if/then block.  This behaves like `if_then()` but instead of
    /// returning the final Instr it emits it.
    fn emit_if_then(
        &mut self,
        pred: ValueId,
        loc: LocId,
        f: impl FnOnce(&mut Self) -> Instr,
    ) -> ValueId;

    /// Build an `is` check.
    fn is(&mut self, vid: ValueId, ty: &EnforceableType, loc: LocId) -> Instr;

    /// Emit an `is` check.  This behaves like `is()` but instead of returning
    /// the final Instr it emits it.
    fn emit_is(&mut self, vid: ValueId, ty: &EnforceableType, loc: LocId) -> ValueId;
}

impl<'a> FuncBuilderEx for FuncBuilder<'a> {
    fn if_then(&mut self, pred: ValueId, loc: LocId, f: impl FnOnce(&mut Self) -> Instr) -> Instr {
        use instr::Predicate;
        let join_bid = self.alloc_bid();
        let true_bid = self.alloc_bid();
        self.emit(Instr::jmp_op(
            pred,
            Predicate::NonZero,
            true_bid,
            join_bid,
            loc,
        ));
        self.start_block(true_bid);
        let instr = f(self);
        self.emit(instr);
        if !self.func.is_terminated(self.cur_bid()) {
            self.emit(Instr::jmp(join_bid, loc));
        }
        self.start_block(join_bid);
        Instr::tombstone()
    }

    fn emit_if_then(
        &mut self,
        pred: ValueId,
        loc: LocId,
        f: impl FnOnce(&mut Self) -> Instr,
    ) -> ValueId {
        let instr = self.if_then(pred, loc, f);
        self.emit(instr)
    }

    fn is(&mut self, vid: ValueId, ety: &EnforceableType, loc: LocId) -> Instr {
        use instr::Hhbc;
        use instr::IsTypeOp;

        if ety.modifiers == TypeConstraintFlags::NoFlags
            || ety.modifiers == TypeConstraintFlags::ExtendedHint
        {
            match ety.ty {
                BaseType::AnyArray => todo!(),
                BaseType::Arraykey => todo!(),
                BaseType::Bool => todo!(),
                BaseType::Class(_cid) => todo!(),
                BaseType::Classname => todo!(),
                BaseType::Darray => todo!(),
                BaseType::Dict => todo!(),
                BaseType::Float => todo!(),
                BaseType::Int => Instr::Hhbc(Hhbc::IsTypeC(vid, IsTypeOp::Int, loc)),
                BaseType::Keyset => todo!(),
                BaseType::Mixed => todo!(),
                BaseType::None => todo!(),
                BaseType::Nonnull => todo!(),
                BaseType::Noreturn => todo!(),
                BaseType::Nothing => todo!(),
                BaseType::Null => todo!(),
                BaseType::Num => todo!(),
                BaseType::Resource => todo!(),
                BaseType::String => todo!(),
                BaseType::This => todo!(),
                BaseType::Typename => todo!(),
                BaseType::Varray => todo!(),
                BaseType::VarrayOrDarray => todo!(),
                BaseType::Vec => todo!(),
                BaseType::VecOrDict => todo!(),
                BaseType::Void => todo!(),
            }
        } else {
            todo!("Unhandled modifiers: {:?}", ety.modifiers);
        }
    }

    fn emit_is(&mut self, vid: ValueId, ty: &EnforceableType, loc: LocId) -> ValueId {
        let instr = self.is(vid, ty, loc);
        self.emit(instr)
    }
}

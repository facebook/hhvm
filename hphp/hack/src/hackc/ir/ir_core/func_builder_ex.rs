// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use crate::instr;
use crate::instr::Hhbc;
use crate::type_struct::TypeStruct;
use crate::BaseType;
use crate::Constant;
use crate::EnforceableType;
use crate::FuncBuilder;
use crate::FunctionId;
use crate::Instr;
use crate::IsTypeOp;
use crate::LocId;
use crate::TypeConstraintFlags;
use crate::TypeStructResolveOp;
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

    fn todo_fake_instr(&mut self, reason: &str, loc: LocId) -> Instr;

    fn emit_todo_fake_instr(&mut self, reason: &str, loc: LocId) -> ValueId;
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
        if ety.modifiers == TypeConstraintFlags::NoFlags
            || ety.modifiers == TypeConstraintFlags::ExtendedHint
        {
            match ety.ty {
                BaseType::AnyArray => self.todo_fake_instr("BaseType::AnyArray", loc),
                BaseType::Arraykey => self.todo_fake_instr("BaseType::Arraykey", loc),
                BaseType::Bool => self.todo_fake_instr("BaseType::Bool", loc),
                BaseType::Class(cid) => {
                    let constant = Constant::Array(Arc::new(
                        TypeStruct::Unresolved(cid).into_typed_value(&self.strings),
                    ));
                    let adata = self.emit_constant(constant);
                    Instr::Hhbc(Hhbc::IsTypeStructC(
                        [vid, adata],
                        TypeStructResolveOp::Resolve,
                        loc,
                    ))
                }
                BaseType::Classname => self.todo_fake_instr("BaseType::Classname", loc),
                BaseType::Darray => self.todo_fake_instr("BaseType::Darray", loc),
                BaseType::Dict => self.todo_fake_instr("BaseType::Dict", loc),
                BaseType::Float => self.todo_fake_instr("BaseType::Float", loc),
                BaseType::Int => Instr::Hhbc(Hhbc::IsTypeC(vid, IsTypeOp::Int, loc)),
                BaseType::Keyset => self.todo_fake_instr("BaseType::Keyset", loc),
                BaseType::Mixed => self.todo_fake_instr("BaseType::Mixed", loc),
                BaseType::None => self.todo_fake_instr("BaseType::None", loc),
                BaseType::Nonnull => self.todo_fake_instr("BaseType::Nonnull", loc),
                BaseType::Noreturn => self.todo_fake_instr("BaseType::Noreturn", loc),
                BaseType::Nothing => self.todo_fake_instr("BaseType::Nothing", loc),
                BaseType::Null => self.todo_fake_instr("BaseType::Null", loc),
                BaseType::Num => self.todo_fake_instr("BaseType::Num", loc),
                BaseType::RawPtr(_) => self.todo_fake_instr("BaseType::RawPtr(_)", loc),
                BaseType::Resource => self.todo_fake_instr("BaseType::Resource", loc),
                BaseType::String => Instr::Hhbc(Hhbc::IsTypeC(vid, IsTypeOp::Str, loc)),
                BaseType::This => Instr::Hhbc(Hhbc::IsLateBoundCls(vid, loc)),
                BaseType::Typename => self.todo_fake_instr("BaseType::Typename", loc),
                BaseType::Varray => self.todo_fake_instr("BaseType::Varray", loc),
                BaseType::VarrayOrDarray => self.todo_fake_instr("BaseType::VarrayOrDarray", loc),
                BaseType::Vec => self.todo_fake_instr("BaseType::Vec", loc),
                BaseType::VecOrDict => self.todo_fake_instr("BaseType::VecOrDict", loc),
                BaseType::Void => self.todo_fake_instr("BaseType::Void", loc),
            }
        } else {
            self.todo_fake_instr(&format!("Unhandled modifiers: {:?}", ety.modifiers), loc)
        }
    }

    fn emit_is(&mut self, vid: ValueId, ty: &EnforceableType, loc: LocId) -> ValueId {
        let instr = self.is(vid, ty, loc);
        self.emit(instr)
    }

    fn todo_fake_instr(&mut self, reason: &str, loc: LocId) -> Instr {
        let op = self.emit_constant(Constant::String(self.strings.intern_str(reason)));
        let fid = FunctionId::from_str("todo", &self.strings);
        Instr::simple_call(fid, &[op], loc)
    }

    fn emit_todo_fake_instr(&mut self, reason: &str, loc: LocId) -> ValueId {
        let instr = self.todo_fake_instr(reason, loc);
        self.emit(instr)
    }
}

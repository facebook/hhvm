// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use crate::instr::Hhbc;
use crate::instr::Predicate;
use crate::type_struct::TypeStruct;
use crate::BaseType;
use crate::Constant;
use crate::EnforceableType;
use crate::FuncBuilder;
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
    fn emit_if_then(&mut self, pred: ValueId, loc: LocId, f: impl FnOnce(&mut Self));

    /// Build a conditional:
    ///     pred ? b() : c()
    ///
    /// Note that although the callback only returns a single Instr it can emit
    /// as many instrs as it wants simply returning the final one. In addition
    /// it can return a tombstone to indicate that it already emitted everything
    /// it needed.
    fn emit_if_then_else(
        &mut self,
        pred: ValueId,
        loc: LocId,
        f_true: impl FnOnce(&mut Self) -> Instr,
        f_false: impl FnOnce(&mut Self) -> Instr,
    ) -> ValueId;

    /// Emit a constant string.
    fn emit_string(&mut self, s: &str) -> ValueId;

    /// Build an `is` check.
    fn is(&mut self, vid: ValueId, ty: &EnforceableType, loc: LocId) -> Instr;

    /// Emit an `is` check.  This behaves like `is()` but instead of returning
    /// the final Instr it emits it.
    fn emit_is(&mut self, vid: ValueId, ty: &EnforceableType, loc: LocId) -> ValueId;
}

impl<'a> FuncBuilderEx for FuncBuilder<'a> {
    fn emit_if_then(&mut self, pred: ValueId, loc: LocId, f: impl FnOnce(&mut Self)) {
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
        f(self);
        if !self.func.is_terminated(self.cur_bid()) {
            self.emit(Instr::jmp(join_bid, loc));
        }
        self.start_block(join_bid);
    }

    fn emit_if_then_else(
        &mut self,
        pred: ValueId,
        loc: LocId,
        f_true: impl FnOnce(&mut Self) -> Instr,
        f_false: impl FnOnce(&mut Self) -> Instr,
    ) -> ValueId {
        // b_true:
        //   arg = f_true()
        //   jmp b_join(arg)
        // b_false:
        //   arg = f_false()
        //   jmp b_join(arg)
        // b_join(arg):

        let mut join_bid = None;
        let true_bid = self.alloc_bid();
        let false_bid = self.alloc_bid();
        self.emit(Instr::jmp_op(
            pred,
            Predicate::NonZero,
            true_bid,
            false_bid,
            loc,
        ));

        self.start_block(true_bid);
        let instr = f_true(self);
        if !instr.is_tombstone() {
            let terminated = matches!(instr, Instr::Terminator(_));
            let arg = self.emit(instr);
            if !terminated {
                let target = self.alloc_bid();
                join_bid = Some(target);
                self.emit(Instr::jmp_args(target, &[arg], loc));
            }
        }

        self.start_block(false_bid);
        let instr = f_false(self);
        if !instr.is_tombstone() {
            let terminated = matches!(instr, Instr::Terminator(_));
            let arg = self.emit(instr);
            if !terminated {
                let target = join_bid.get_or_insert_with(|| self.alloc_bid());
                self.emit(Instr::jmp_args(*target, &[arg], loc));
            }
        }

        if let Some(join_bid) = join_bid {
            self.start_block(join_bid);
            self.alloc_param()
        } else {
            ValueId::none()
        }
    }

    fn emit_string(&mut self, s: &str) -> ValueId {
        self.emit_constant(Constant::String(self.strings.intern_str(s)))
    }

    fn is(&mut self, vid: ValueId, ety: &EnforceableType, loc: LocId) -> Instr {
        let mut modifiers = ety.modifiers;

        if modifiers.contains(TypeConstraintFlags::Nullable) {
            let is1 = self.is(vid, &EnforceableType::null(), loc);
            let is1 = self.emit(is1);
            let ety = EnforceableType {
                ty: ety.ty,
                modifiers: modifiers
                    - TypeConstraintFlags::Nullable
                    - TypeConstraintFlags::DisplayNullable,
            };
            return Instr::copy(self.emit_if_then_else(
                is1,
                loc,
                |builder| Instr::copy(builder.emit_constant(Constant::Bool(true))),
                |builder| builder.is(vid, &ety, loc),
            ));
        }

        // (I think) ExtendedHint is used to enable non-PHP type flags - which
        // is now standard so we can just ignore it.
        modifiers -= TypeConstraintFlags::ExtendedHint;
        // We're just trying to do an 'is' check - we don't care if the context
        // is a TypeConstant.
        modifiers -= TypeConstraintFlags::TypeConstant;

        // TypeVars can't be 'is' texted because they're non-reified types.
        assert!(!modifiers.contains(TypeConstraintFlags::TypeVar));

        if modifiers == TypeConstraintFlags::NoFlags {
            fn is_type_op(op: IsTypeOp, vid: ValueId, loc: LocId) -> Instr {
                Instr::Hhbc(Hhbc::IsTypeC(vid, op, loc))
            }

            fn is_type_op2(
                op1: IsTypeOp,
                op2: IsTypeOp,
                builder: &mut FuncBuilder<'_>,
                vid: ValueId,
                loc: LocId,
            ) -> Instr {
                let is1 = builder.emit(is_type_op(op1, vid, loc));
                Instr::copy(builder.emit_if_then_else(
                    is1,
                    loc,
                    |builder| Instr::copy(builder.emit_constant(Constant::Bool(true))),
                    |_| is_type_op(op2, vid, loc),
                ))
            }

            match ety.ty {
                BaseType::AnyArray => is_type_op(IsTypeOp::ArrLike, vid, loc),
                BaseType::Arraykey => is_type_op2(IsTypeOp::Str, IsTypeOp::Int, self, vid, loc),
                BaseType::Bool => is_type_op(IsTypeOp::Bool, vid, loc),
                BaseType::Classname => is_type_op(IsTypeOp::Class, vid, loc),
                BaseType::Darray => is_type_op(IsTypeOp::Dict, vid, loc),
                BaseType::Dict => is_type_op(IsTypeOp::Dict, vid, loc),
                BaseType::Float => is_type_op(IsTypeOp::Dbl, vid, loc),
                BaseType::Int => is_type_op(IsTypeOp::Int, vid, loc),
                BaseType::Keyset => is_type_op(IsTypeOp::Keyset, vid, loc),
                BaseType::Mixed | BaseType::None => {
                    Instr::copy(self.emit_constant(Constant::Bool(true)))
                }
                BaseType::Nothing => Instr::copy(self.emit_constant(Constant::Bool(false))),
                BaseType::Null => is_type_op(IsTypeOp::Null, vid, loc),
                BaseType::Num => is_type_op2(IsTypeOp::Int, IsTypeOp::Dbl, self, vid, loc),
                BaseType::Resource => is_type_op(IsTypeOp::Res, vid, loc),
                BaseType::String => is_type_op(IsTypeOp::Str, vid, loc),
                BaseType::This => Instr::Hhbc(Hhbc::IsLateBoundCls(vid, loc)),
                BaseType::Varray => is_type_op(IsTypeOp::Vec, vid, loc),
                BaseType::VarrayOrDarray => {
                    is_type_op2(IsTypeOp::Dict, IsTypeOp::Vec, self, vid, loc)
                }
                BaseType::Vec => is_type_op(IsTypeOp::Vec, vid, loc),
                BaseType::VecOrDict => is_type_op(IsTypeOp::Dict, vid, loc),

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
                BaseType::Nonnull => {
                    let iid = self.emit(Instr::Hhbc(Hhbc::IsTypeC(vid, IsTypeOp::Null, loc)));
                    Instr::Hhbc(Hhbc::Not(iid, loc))
                }

                BaseType::Noreturn => panic!("Unable to perform 'is' on 'noreturn'"),
                BaseType::Typename => panic!("Unable to perform 'is' on 'typename'"),
                BaseType::Void => panic!("Unable to perform 'is' on 'void'"),
            }
        } else {
            todo!("Unhandled modifiers: {:?}", modifiers);
        }
    }

    fn emit_is(&mut self, vid: ValueId, ty: &EnforceableType, loc: LocId) -> ValueId {
        let instr = self.is(vid, ty, loc);
        self.emit(instr)
    }
}

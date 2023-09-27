// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::BaseType;
use ir::EnforceableType;
use ir::StringInterner;
use ir::TypeConstraintFlags;

use crate::mangle::TypeName;
use crate::textual;

pub(crate) fn convert_ty(ty: &EnforceableType, strings: &StringInterner) -> textual::Ty {
    let mut base = convert_base(&ty.ty);

    let mut modifiers = ty.modifiers;

    // ExtendedHint does nothing interesting.
    modifiers -= TypeConstraintFlags::ExtendedHint;
    // DisplayNullable does nothing interesting.
    modifiers -= TypeConstraintFlags::DisplayNullable;
    // TypeVar doesn't add any interesting context.
    modifiers -= TypeConstraintFlags::TypeVar;

    // All textual boxed types are nullable.
    modifiers -= TypeConstraintFlags::Nullable;

    if modifiers.contains(TypeConstraintFlags::TypeConstant) {
        // A type constant is treated as a raw mixed.
        return textual::Ty::mixed_ptr();
    }

    if modifiers != TypeConstraintFlags::NoFlags {
        textual_todo! {
            message = ("MODIFIERS: {modifiers:?}\n{}",
                       ir::print::FmtEnforceableType(ty, strings)
            ),
            base = textual::Ty::Type(TypeName::UnmangledRef("TODO_NoFlags"));
        }
    }

    base
}

fn convert_base(ty: &BaseType) -> textual::Ty {
    match ty {
        BaseType::Arraykey => textual::Ty::SpecialPtr(textual::SpecialTy::Arraykey),
        BaseType::Bool => textual::Ty::SpecialPtr(textual::SpecialTy::Bool),
        BaseType::Class(cid) => textual::Ty::named_type_ptr(TypeName::Class(*cid)),
        BaseType::Classname => textual::Ty::named_type_ptr(TypeName::UnmangledRef("Classname")),
        BaseType::Dict => textual::Ty::SpecialPtr(textual::SpecialTy::Dict),
        BaseType::Float => textual::Ty::SpecialPtr(textual::SpecialTy::Float),
        BaseType::Int => textual::Ty::SpecialPtr(textual::SpecialTy::Int),
        BaseType::Keyset => textual::Ty::SpecialPtr(textual::SpecialTy::Keyset),
        BaseType::Null => textual::Ty::VoidPtr,
        BaseType::Num => textual::Ty::SpecialPtr(textual::SpecialTy::Num),
        BaseType::Resource => textual::Ty::named_type_ptr(TypeName::UnmangledRef("HackResource")),
        BaseType::String => textual::Ty::SpecialPtr(textual::SpecialTy::String),
        BaseType::Vec => textual::Ty::SpecialPtr(textual::SpecialTy::Vec),
        // Why VoidPtr? Because in Hack something returning `void` implicitly
        // returns a null.
        BaseType::Void => textual::Ty::VoidPtr,

        BaseType::AnyArray
        | BaseType::Darray
        | BaseType::Varray
        | BaseType::VarrayOrDarray
        | BaseType::VecOrDict => textual::Ty::named_type_ptr(TypeName::UnmangledRef("HackArray")),

        BaseType::Noreturn | BaseType::Nothing => textual::Ty::ptr(textual::Ty::Noreturn),

        BaseType::Mixed
        | BaseType::None
        | BaseType::Nonnull
        | BaseType::This
        | BaseType::Typename => textual::Ty::mixed_ptr(),
    }
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::TypeConstraintFlags;

use crate::textual;

pub(crate) fn convert_ty(ty: ir::EnforceableType) -> textual::Ty {
    let mut base = convert_base(ty.ty);

    let mut modifiers = ty.modifiers;

    // ExtendedHint does nothing interesting.
    if modifiers.contains(TypeConstraintFlags::ExtendedHint) {
        modifiers = modifiers & !TypeConstraintFlags::ExtendedHint;
    }
    if modifiers.contains(TypeConstraintFlags::Nullable) {
        base = textual::Ty::Nullable(Box::new(base));
        modifiers = modifiers & !TypeConstraintFlags::Nullable;
    }

    if modifiers != TypeConstraintFlags::NoFlags {
        todo!("modifiers: {:?}", ty.modifiers);
    }

    base
}

fn convert_base(ty: ir::BaseType) -> textual::Ty {
    use ir::BaseType;
    match ty {
        BaseType::AnyArray => textual::Ty::AnyArray,
        BaseType::Arraykey => textual::Ty::Arraykey,
        BaseType::Bool => tx_ty!(bool),
        BaseType::Classname => textual::Ty::Classname,
        BaseType::Darray => textual::Ty::Darray,
        BaseType::Dict => textual::Ty::Dict,
        BaseType::Float => textual::Ty::Float,
        BaseType::Int => tx_ty!(int),
        BaseType::Keyset => textual::Ty::Keyset,
        BaseType::Mixed => tx_ty!(mixed),
        BaseType::None => textual::Ty::None,
        BaseType::Nonnull => textual::Ty::Nonnull,
        BaseType::Noreturn => tx_ty!(noreturn),
        BaseType::Nothing => textual::Ty::Nothing,
        BaseType::Null => textual::Ty::Null,
        BaseType::Num => textual::Ty::Num,
        BaseType::Resource => textual::Ty::Resource,
        BaseType::String => tx_ty!(string),
        BaseType::This => textual::Ty::This,
        BaseType::Typename => textual::Ty::Typename,
        BaseType::Varray => textual::Ty::Varray,
        BaseType::VarrayOrDarray => textual::Ty::VarrayOrDarray,
        BaseType::Vec => textual::Ty::Vec,
        BaseType::VecOrDict => textual::Ty::VecOrDict,
        BaseType::Void => tx_ty!(*void),
        BaseType::Class(cid) => textual::Ty::Class(cid),
        BaseType::RawPtr(box base) => textual::Ty::Ptr(Box::new(convert_base(base))),
        BaseType::RawType(name) => textual::Ty::Type(name),
    }
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::StringInterner;
use ir::TypeConstraintFlags;
use log::trace;

use crate::class::IsStatic;
use crate::mangle::MangleClass as _;
use crate::textual;

pub(crate) fn convert_ty(ty: ir::EnforceableType, strings: &StringInterner) -> textual::Ty {
    let mut base = convert_base(ty.ty, strings);

    let mut modifiers = ty.modifiers;

    // ExtendedHint does nothing interesting.
    if modifiers.contains(TypeConstraintFlags::ExtendedHint) {
        modifiers = modifiers & !TypeConstraintFlags::ExtendedHint;
    }
    if modifiers.contains(TypeConstraintFlags::Nullable) {
        // All textual boxed types are nullable.
        //base = textual::Ty::Nullable(Box::new(base));
        modifiers = modifiers & !TypeConstraintFlags::Nullable;
    }

    if modifiers != TypeConstraintFlags::NoFlags {
        textual_todo! {
            base = textual::Ty::Type("TODO_NoFlags".to_string());
        }
    }

    base
}

fn convert_base(ty: ir::BaseType, strings: &StringInterner) -> textual::Ty {
    use ir::BaseType;
    match ty {
        BaseType::Void => textual::Ty::Void,
        BaseType::Class(cid) => textual::Ty::Type(cid.mangle_class(IsStatic::NonStatic, strings)),
        BaseType::RawPtr(box base) => textual::Ty::Ptr(Box::new(convert_base(base, strings))),
        _ => {
            trace!("BaseType: {ty:?}");
            textual_todo! {
                textual::Ty::Int
            }
        }
    }
}

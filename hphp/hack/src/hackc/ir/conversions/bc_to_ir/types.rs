// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Str;
use hhbc::Constraint;
use hhbc::TypeInfo;
use hhvm_types_ffi::ffi::TypeConstraintFlags;
use lazy_static::lazy_static;

lazy_static! {
    static ref BUILTIN_NAME_VOID: Str<'static> = Str::new(br"HH\void");
}

pub(crate) fn convert_type<'a>(ty: &TypeInfo<'a>) -> ir::Type<'a> {
    if !ty.type_constraint.flags.is_empty() {
        let inner = convert_type(&TypeInfo {
            user_type: ty.user_type,
            type_constraint: Constraint {
                name: ty.type_constraint.name,
                flags: TypeConstraintFlags::NoFlags,
            },
        });
        return ir::Type::Flags(ty.type_constraint.flags, Box::new(inner));
    }

    match (ty.user_type, ty.type_constraint.name) {
        (Maybe::Just(user_type), Maybe::Just(constraint_name)) => {
            if user_type == constraint_name {
                ir::Type::User(user_type)
            } else {
                ir::Type::UserWithConstraint(user_type, constraint_name)
            }
        }
        (Maybe::Nothing, Maybe::Just(constraint_name)) => {
            todo!("unhandled: {}", constraint_name.unsafe_as_str());
        }
        (Maybe::Just(user_type), Maybe::Nothing) => {
            if user_type.is_empty() {
                ir::Type::Empty
            } else if user_type == *BUILTIN_NAME_VOID {
                ir::Type::Void
            } else {
                ir::Type::UserNoConstraint(user_type)
            }
        }
        (Maybe::Nothing, Maybe::Nothing) => ir::Type::None,
    }
}

pub(crate) fn convert_maybe_type<'a>(ty: Maybe<&TypeInfo<'a>>) -> ir::Type<'a> {
    match ty {
        Maybe::Just(ty) => convert_type(ty),
        Maybe::Nothing => ir::Type::None,
    }
}

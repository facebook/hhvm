// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Maybe::Just;
use ffi::Str;
use ffi::Vector;
use hhvm_types_ffi::ffi::TypeConstraintFlags;
use intern::string::StringId;
use serde::Serialize;

/// Type info has additional optional user type
#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct TypeInfo<'arena> {
    pub user_type: Maybe<Str<'arena>>,
    pub type_constraint: Constraint,
}

#[derive(Clone, Default, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Constraint {
    pub name: Maybe<StringId>,
    pub flags: TypeConstraintFlags,
}

#[derive(Debug, Default, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct UpperBound<'arena> {
    pub name: Str<'arena>,
    pub bounds: Vector<TypeInfo<'arena>>,
}

impl Constraint {
    pub fn new(name: Maybe<StringId>, flags: TypeConstraintFlags) -> Self {
        Self { name, flags }
    }

    pub fn intern(name: impl AsRef<str>, flags: TypeConstraintFlags) -> Self {
        Self {
            name: Just(intern::string::intern(name.as_ref())),
            flags,
        }
    }
}

impl<'arena> TypeInfo<'arena> {
    pub fn make(user_type: Maybe<Str<'arena>>, type_constraint: Constraint) -> Self {
        Self {
            user_type,
            type_constraint,
        }
    }

    pub fn make_empty() -> TypeInfo<'arena> {
        TypeInfo::make(Just("".into()), Constraint::default())
    }

    pub fn has_type_constraint(&self) -> bool {
        self.type_constraint.name.is_just()
    }
}

#[cfg(test)]
mod test {
    #[test]
    fn test_constraint_flags_to_string_called_by_hhbc_hhas() {
        use hhvm_types_ffi::ffi::TypeConstraintFlags;
        let typevar_and_soft = TypeConstraintFlags::TypeVar | TypeConstraintFlags::Soft;
        assert_eq!("type_var soft", typevar_and_soft.to_string());
    }
}

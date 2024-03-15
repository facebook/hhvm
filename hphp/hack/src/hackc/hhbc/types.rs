// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Maybe::Just;
use ffi::Vector;
use hhvm_types_ffi::ffi::TypeConstraintFlags;
use intern::string::StringId;
use serde::Serialize;

/// Type info has additional optional user type
#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct TypeInfo {
    pub user_type: Maybe<StringId>,
    pub type_constraint: Constraint,
}

impl TypeInfo {
    pub fn new(user_type: Maybe<StringId>, type_constraint: Constraint) -> Self {
        Self {
            user_type,
            type_constraint,
        }
    }

    pub fn has_type_constraint(&self) -> bool {
        self.type_constraint.name.is_just()
    }
}

impl Default for TypeInfo {
    fn default() -> Self {
        Self::new(Just(StringId::EMPTY), Constraint::default())
    }
}

#[derive(Clone, Default, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Constraint {
    pub name: Maybe<StringId>,
    pub flags: TypeConstraintFlags,
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

#[derive(Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct UpperBound {
    pub name: StringId,
    pub bounds: Vector<TypeInfo>,
}

impl Default for UpperBound {
    fn default() -> Self {
        Self {
            name: StringId::EMPTY,
            bounds: Default::default(),
        }
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

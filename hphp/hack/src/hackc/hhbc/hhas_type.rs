// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Maybe, Maybe::Just, Str};
use hhvm_types_ffi::ffi::TypeConstraintFlags;

/// Type info has additional optional user type
#[derive(Clone, Debug, Eq, PartialEq)]
#[repr(C)]
pub struct HhasTypeInfo<'arena> {
    pub user_type: Maybe<Str<'arena>>,
    pub type_constraint: Constraint<'arena>,
}

#[derive(Clone, Default, Debug, Eq, PartialEq)]
#[repr(C)]
pub struct Constraint<'arena> {
    pub name: Maybe<Str<'arena>>,
    pub flags: TypeConstraintFlags,
}

impl<'arena> Constraint<'arena> {
    pub fn make(name: Maybe<Str<'arena>>, flags: TypeConstraintFlags) -> Self {
        Self { name, flags }
    }

    pub fn make_with_raw_str(
        alloc: &'arena bumpalo::Bump,
        name: &str,
        flags: TypeConstraintFlags,
    ) -> Self {
        Constraint::make(Just(Str::new_str(alloc, name)), flags)
    }
}

impl<'arena> HhasTypeInfo<'arena> {
    pub fn make(user_type: Maybe<Str<'arena>>, type_constraint: Constraint<'arena>) -> Self {
        Self {
            user_type,
            type_constraint,
        }
    }

    pub fn make_empty() -> HhasTypeInfo<'arena> {
        HhasTypeInfo::make(Just("".into()), Constraint::default())
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

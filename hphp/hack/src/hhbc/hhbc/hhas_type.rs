// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Maybe, Maybe::*, Str};

/// Type info has additional optional user type
#[derive(Clone, Debug)]
#[repr(C)]
pub struct HhasTypeInfo<'arena> {
    pub user_type: Maybe<Str<'arena>>,
    pub type_constraint: constraint::Constraint<'arena>,
}

#[allow(dead_code)]
pub mod constraint {
    use bitflags::bitflags;
    use ffi::{Maybe, Maybe::Just, Str};

    #[derive(Clone, Default, Debug)]
    #[repr(C)]
    pub struct Constraint<'arena> {
        pub name: Maybe<Str<'arena>>,
        pub flags: ConstraintFlags,
    }

    bitflags! {
        #[derive(Default)]
        #[repr(C)]
        pub struct ConstraintFlags: u8 {
            const NULLABLE =         0b0000_0001;
            const EXTENDED_HINT =    0b0000_0100;
            const TYPE_VAR =         0b0000_1000;
            const SOFT =             0b0001_0000;
            const TYPE_CONSTANT =    0b0010_0000;
            const DISPLAY_NULLABLE = 0b0100_0000;
            const UPPERBOUND =       0b1000_0000;
        }
    }

    /// Implicitly provides a to_string() method consistent with the one
    /// used in hhbc_hhas: i.e., lowercased flag names separated by space.
    impl std::fmt::Display for ConstraintFlags {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            let set_names: String = format!("{:?}", self)
                .split(" | ")
                .map(|s| s.to_lowercase())
                .collect::<Vec<_>>()
                .join(" ");
            write!(f, "{}", set_names)
        }
    }

    impl<'arena> Constraint<'arena> {
        pub fn make(name: Maybe<Str<'arena>>, flags: ConstraintFlags) -> Self {
            Constraint { name, flags }
        }

        pub fn make_with_raw_str(
            alloc: &'arena bumpalo::Bump,
            name: &str,
            flags: ConstraintFlags,
        ) -> Self {
            Constraint::make(Just(Str::new_str(alloc, name)), flags)
        }
    }
}

impl<'arena> HhasTypeInfo<'arena> {
    pub fn make(
        user_type: Maybe<Str<'arena>>,
        type_constraint: constraint::Constraint<'arena>,
    ) -> HhasTypeInfo<'arena> {
        HhasTypeInfo {
            user_type,
            type_constraint,
        }
    }

    pub fn make_empty(alloc: &'arena bumpalo::Bump) -> HhasTypeInfo<'arena> {
        HhasTypeInfo::make(
            Just(Str::new_str(alloc, "")),
            constraint::Constraint::default(),
        )
    }

    pub fn has_type_constraint(&self) -> bool {
        self.type_constraint.name.is_just()
    }
}

#[cfg(test)]
mod test {
    #[test]
    fn test_constraint_flags_to_string_called_by_hhbc_hhas() {
        use crate::constraint::ConstraintFlags;
        let typevar_and_soft = ConstraintFlags::TYPE_VAR | ConstraintFlags::SOFT;
        assert_eq!("type_var soft", typevar_and_soft.to_string());
    }
}

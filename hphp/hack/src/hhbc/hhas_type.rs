// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern crate bitflags;

/// Type info has additional optional user type *)
#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct Info {
    pub user_type: Option<String>,
    pub type_constraint: constraint::Type,
}

#[derive(Debug)]
pub struct Constant {
    pub name: String,
    pub initializer: constraint::Type,
}

#[allow(dead_code)]
pub mod constraint {

    use bitflags::bitflags;

    #[derive(Clone, Default, Debug)]
    pub struct Type {
        pub name: Option<String>,
        pub flags: Flags,
    }

    bitflags! {
        #[derive(Default)]
        pub struct Flags: u8 {
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
    impl std::fmt::Display for Flags {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            let set_names: String = format!("{:?}", self)
                .split(" | ")
                .map(|s| s.to_lowercase())
                .collect::<Vec<_>>()
                .join(" ");
            write!(f, "{}", set_names.to_string())
        }
    }

    impl Type {
        pub fn make(name: Option<String>, flags: Flags) -> Type {
            Type { name, flags }
        }

        pub fn make_with_raw_str(name: &str, flags: Flags) -> Type {
            Self::make(Some(String::from(name)), flags)
        }
    }
}

impl Info {
    pub fn make(user_type: Option<String>, type_constraint: constraint::Type) -> Info {
        Info {
            user_type,
            type_constraint,
        }
    }

    pub fn make_empty() -> Info {
        Info::make(Some(String::from("")), constraint::Type::default())
    }

    pub fn has_type_constraint(&self) -> bool {
        self.type_constraint.name.is_some()
    }
}

impl Constant {
    pub fn make(name: String, initializer: constraint::Type) -> Constant {
        Constant { name, initializer }
    }
}

#[cfg(test)]
mod test {

    #[test]
    fn test_constraint_flags_to_string_called_by_hhbc_hhas() {
        use crate::constraint::Flags;
        let typevar_and_soft = Flags::TYPE_VAR | Flags::SOFT;
        assert_eq!("type_var soft", typevar_and_soft.to_string());
    }
}

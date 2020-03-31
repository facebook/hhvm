// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_string_utils_rust::{mangle, strip_global_ns};

use std::convert::From;

pub trait Id<'a>: Sized {
    const MANGLE: bool;

    fn to_raw_string(&self) -> &str;

    fn from_ast_name(s: &'a str) -> Self
    where
        Self: From<&'a str>,
    {
        strip_global_ns(s).into()
    }

    fn to_unmangled_str(&self) -> String {
        if Self::MANGLE {
            panic!("TOOD(hrust) port unmangle XHP in string_utils")
        } else {
            self.to_raw_string().into()
        }
    }
}

/// An Id impl with hidden representation that provides conversion methods:
/// - from &str or String via .into() (i.e., from_raw_string in OCaml)
/// - to_raw_string
macro_rules! impl_id {
    ($type: ident, mangle = $mangle:expr, { $($trait_impl: tt)* }) => {
        use std::borrow::Cow;

        #[derive(Clone)]
        pub struct $type<'a>(Cow<'a, str>);

        impl<'a> Default for $type<'a> {
            fn default() -> Self {
                Self("".into())
            }
        }

        impl<'a> crate::Id<'a> for $type<'a> {
            const MANGLE: bool = $mangle;

            fn to_raw_string(&self) -> &str {
                &self.0
            }

            $( $trait_impl )*
        }

        // TODO(hrust) remove after migration; .into() is more general, it accepts
        // - &str
        // - String
        // - Cow<str>
        pub fn from_raw_string<'a>(s: &'a str) -> $type<'a> {
            $type(s.into())
        }

        impl<'a, S> From<S> for $type<'a>
        where
            S: Into<Cow<'a, str>>,
        {
            fn from(s: S) -> $type<'a> {
                $type(s.into())
            }
        }

        impl<'a> std::fmt::Debug for $type<'a> {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                use crate::Id;
                write!(f, "{}({})", module_path!(), self.to_raw_string())
            }
        }

        // Note: more efficient than to_raw_string().to_owned() if Cow is owned
        impl Into<String> for $type<'_> {
            fn into(self) -> String {
                self.0.into_owned()
            }
        }
    };
}

macro_rules! impl_add_suffix {
    ($type: ident) => {
        // ok (multiple impl Struct blocks are allowed if needed)
        impl<'a> $type<'a> {
            pub fn add_suffix(&mut self, suffix: &str) {
                self.0.to_mut().push_str(suffix)
            }
        }
    };
}

pub mod class {
    use super::*;

    impl_id!(Type, mangle = true, {
        fn from_ast_name(s: &'a str) -> Type<'a> {
            strip_global_ns(&mangle(s.to_string())).to_string().into()
        }
    });

    impl<'a> Type<'a> {
        pub fn from_ast_name_and_mangle(s: impl Into<String>) -> Self {
            strip_global_ns(&mangle(s.into())).to_string().into()
        }
    }
}

pub mod prop {
    impl_id!(Type, mangle = false, {});
    impl_add_suffix!(Type);
}

pub mod method {
    impl_id!(Type, mangle = false, {});
    impl_add_suffix!(Type);
}

pub mod function {
    impl_id!(Type, mangle = false, {});
    impl_add_suffix!(Type);
}

// escape reserved keyword via r#
pub mod r#const {
    impl_id!(Type, mangle = false, {});
}

pub mod record {
    impl_id!(Type, mangle = false, {});
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_from_to_raw_string() {
        assert_eq!("Foo", class::from_raw_string("Foo").to_raw_string());
    }

    #[test]
    fn test_add_suffix() {
        let mut id: prop::Type = "Some".into();
        id.add_suffix("Property");
        assert_eq!("SomeProperty", id.to_raw_string());
    }

    #[test]
    fn test_from_ast_name() {
        let id: method::Type = method::Type::from_ast_name("meth");
        assert_eq!("meth", id.to_raw_string());
    }
}

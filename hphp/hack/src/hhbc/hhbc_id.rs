// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_string_utils_rust::{is_xhp, mangle_xhp_id, strip_global_ns};

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

    fn map_namespace_env(&self, ns: NamespaceEnv) -> NamespaceEnv {
        ns
    }

    fn elaborate(&self) -> Self
    where
        Self: From<String> + Elaborable,
    {
        let mut unstripped = Self::ELABORATE_KIND.elaborate_id(self.to_raw_string()).1;
        if Self::MANGLE {
            unstripped = mangle_xhp_id(self.to_raw_string().to_owned());
        }
        strip_global_ns(&unstripped).to_owned().into()
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

pub trait Elaborable {
    const ELABORATE_KIND: ElaborateKind;
}

/// Helper macro to avoid the following boilerplate in each module:
/// ```
/// use super::*;
/// impl<'a> Elaborable for Type<'a> {
///    const ELABORATE_KIND: ElaborateKind = ElaborateKind::Fun;
/// }
/// ```
macro_rules! impl_elaborable {
    ($type: ident, $kind: path, { $($trait_impl: tt)* }) => {
        use crate::{Elaborable, ElaborateKind};
        impl<'a> Elaborable for $type<'a> {
            const ELABORATE_KIND: ElaborateKind = $kind;
            $( $trait_impl )*
        }
    };
}

pub mod class {
    use super::*;

    impl_id!(Type, mangle = true, {
        fn from_ast_name(s: &'a str) -> Type<'a> {
            if true
            /* TODO(hrust) port new Hh_autoimport logic */
            {
                s.into()
            } else {
                let mut ret = "HH\\".to_owned();
                ret.push_str(s);
                ret.into()
            }
        }

        fn map_namespace_env(&self, ns: NamespaceEnv) -> super::NamespaceEnv {
            if is_xhp(self.to_raw_string()) {
                // TODO(hrust) namespace_env::empty_from_env(ns)
                ns
            } else {
                ns
            }
        }
    });
    impl_elaborable!(Type, ElaborateKind::Class, {});
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
    impl_elaborable!(Type, ElaborateKind::Fun, {});
}

// escape reserved keyword via r#
pub mod r#const {
    impl_id!(Type, mangle = false, {});
    impl_elaborable!(Type, ElaborateKind::Const, {});
}

pub mod record {
    impl_id!(Type, mangle = false, {});
    impl_elaborable!(Type, ElaborateKind::Record, {});
}

// TODO(hrust) port namespaces to Rust, and move this there
type NamespaceEnv = ();
pub enum ElaborateKind {
    Fun,
    Class,
    Record,
    Const,
}
impl ElaborateKind {
    pub fn elaborate_id(&self, _id: &str) -> ((), String) {
        ((), "not yet implemented".into())
    }
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

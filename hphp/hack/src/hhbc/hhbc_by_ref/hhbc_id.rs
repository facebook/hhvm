// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub trait Id<'arena>: Sized {
    const MANGLE: bool;

    fn to_raw_string(&self) -> &'arena str;

    fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> Self;

    fn to_unmangled_str(&self) -> std::borrow::Cow<'arena, str> {
        if Self::MANGLE {
            std::borrow::Cow::from(hhbc_by_ref_hhbc_string_utils::unmangle(
                self.to_raw_string().into(),
            ))
        } else {
            std::borrow::Cow::from(self.to_raw_string())
        }
    }
}

//Note: `$type<'arena>` is a newtype over a `&'arena str`
// (`class::Type<'arena>`, `function::Type<'arena>`, ...). We
//intend that these types borrow strings stored in `InstrSeq`
//arenas.

//Note: We manually write these definitions "outside" `impl_id` macro
//so as to allow `cbindgen` to see the definitions in order
//that it may generate compatible C structs.

/// An Id impl with hidden representation that provides conversion methods:
/// - from &str or String via .into() (i.e., from_raw_string in OCaml)
/// - to_raw_string
macro_rules! impl_id {
    ($type: ident, mangle = $mangle:expr, { $($trait_impl: tt)* }) => {

        impl<'arena> crate::Id<'arena> for $type<'arena> {
            const MANGLE: bool = $mangle;

            fn to_raw_string(&self) -> &'arena str {
                self.0.as_str()
            }

            $( $trait_impl )*
        }

        pub fn from_raw_string<'arena>(alloc: &'arena bumpalo::Bump, s: &str) -> $type<'arena> {
            (alloc, s).into()
        }

        #[allow(dead_code)]
        fn from_raw_string_with_suffix<'arena>(alloc: &'arena bumpalo::Bump, s: &str, suffix: &str) -> $type<'arena> {
            let mut r = bumpalo::collections::String::<'arena>::with_capacity_in(
                s.len() + suffix.len(),
                alloc,
            );
            r.push_str(s);
            r.push_str(suffix);
            $type(ffi::Slice::new(r.into_bump_str().as_bytes()))
        }

        impl<'arena, 'a, S> std::convert::From<(&'arena bumpalo::Bump, S)> for $type<'arena>
        where
            S: std::convert::Into<&'a str>,
        {
            fn from((alloc, s): (&'arena bumpalo::Bump, S)) -> $type<'arena> {
                $type(Str::new(bumpalo::collections::String::from_str_in(s.into(), alloc).into_bump_str().as_bytes()))
            }
        }

        impl<'arena> std::fmt::Debug for $type<'arena> {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                use crate::Id;
                write!(f, "{}({})", module_path!(), self.to_raw_string())
            }
        }

        impl<'arena> Into<std::string::String> for $type<'arena> {
            fn into(self) -> std::string::String {
                self.0.as_str().into()
            }
        }
  }
}

macro_rules! impl_add_suffix {
    ($type: ident) => {
        // ok (multiple impl Struct blocks are allowed if needed)
        impl<'arena> $type<'arena> {
            pub fn add_suffix(alloc: &'arena bumpalo::Bump, id: &Self, suffix: &str) -> Self {
                from_raw_string_with_suffix(alloc, id.0.as_str(), suffix)
            }
        }
    };
}

pub mod class {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct Type<'arena>(Str<'arena>);

    impl_id!(Type, mangle = true, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> Type<'arena> {
            (
                alloc,
                hhbc_by_ref_hhbc_string_utils::strip_global_ns(
                    &hhbc_by_ref_hhbc_string_utils::mangle(s.to_string()),
                ),
            )
                .into()
        }
    });

    impl<'arena> Type<'arena> {
        pub fn from_ast_name_and_mangle(
            alloc: &'arena bumpalo::Bump,
            s: impl std::convert::Into<std::string::String>,
        ) -> Self {
            use crate::Id;
            self::Type::<'arena>::from_ast_name(alloc, s.into().as_str())
        }
    }
}

pub mod prop {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct Type<'arena>(Str<'arena>);

    impl_id!(Type, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> Type<'arena> {
            (alloc, hhbc_by_ref_hhbc_string_utils::strip_global_ns(s)).into()
        }
    });
    impl_add_suffix!(Type);
}

pub mod method {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct Type<'arena>(Str<'arena>);

    impl_id!(Type, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> Type<'arena> {
            (alloc, hhbc_by_ref_hhbc_string_utils::strip_global_ns(s)).into()
        }
    });
    impl_add_suffix!(Type);

    impl<'arena> Type<'arena> {
        pub fn from_ast_name_and_suffix(
            alloc: &'arena bumpalo::Bump,
            s: &str,
            suffix: &str,
        ) -> Self {
            from_raw_string_with_suffix(
                alloc,
                hhbc_by_ref_hhbc_string_utils::strip_global_ns(s),
                suffix,
            )
        }
    }
}

pub mod function {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct Type<'arena>(Str<'arena>);

    impl_id!(Type, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> Type<'arena> {
            (alloc, hhbc_by_ref_hhbc_string_utils::strip_global_ns(s)).into()
        }
    });
    impl_add_suffix!(Type);
}

// escape reserved keyword via r#
pub mod r#const {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct Type<'arena>(Str<'arena>);

    impl_id!(Type, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> Type<'arena> {
            (alloc, hhbc_by_ref_hhbc_string_utils::strip_global_ns(s)).into()
        }
    });
}

pub mod record {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct Type<'arena>(Str<'arena>);

    impl_id!(Type, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> Type<'arena> {
            (alloc, hhbc_by_ref_hhbc_string_utils::strip_global_ns(s)).into()
        }
    });
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_from_to_raw_string() {
        let alloc = bumpalo::Bump::new();
        assert_eq!("Foo", class::from_raw_string(&alloc, "Foo").to_raw_string());
    }

    #[test]
    fn test_add_suffix() {
        let alloc = bumpalo::Bump::new();
        let id: prop::Type = (&alloc, "Some").into();
        let id = prop::Type::add_suffix(&alloc, &id, "Property");
        assert_eq!("SomeProperty", id.to_raw_string());
    }

    #[test]
    fn test_from_ast_name() {
        let alloc = bumpalo::Bump::new();
        let id: method::Type = method::Type::from_ast_name(&alloc, "meth");
        assert_eq!("meth", id.to_raw_string());
    }
}

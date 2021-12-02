// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub trait Id<'arena>: Sized {
    const MANGLE: bool;

    fn to_raw_string(&self) -> &'arena str;

    fn to_raw_ffi_str(&self) -> ffi::Str<'arena>;

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
                self.0.unsafe_as_str()
            }

            fn to_raw_ffi_str(&self) -> ffi::Str<'arena> {
                self.0
            }

            $( $trait_impl )*
        }

        pub fn from_raw_string<'arena>(alloc: &'arena bumpalo::Bump, s: &str) -> $type<'arena> {
            $type(Str::new_str(alloc, s))
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

        impl<'arena> std::fmt::Debug for $type<'arena> {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                use crate::Id;
                write!(f, "{}({})", module_path!(), self.to_raw_string())
            }
        }

        impl<'arena> Into<std::string::String> for $type<'arena> {
            fn into(self) -> std::string::String {
                self.0.unsafe_as_str().into()
            }
        }
  }
}

macro_rules! impl_add_suffix {
    ($type: ident) => {
        // ok (multiple impl Struct blocks are allowed if needed)
        impl<'arena> $type<'arena> {
            pub fn add_suffix(alloc: &'arena bumpalo::Bump, id: &Self, suffix: &str) -> Self {
                from_raw_string_with_suffix(alloc, id.0.unsafe_as_str(), suffix)
            }
        }
    };
}

pub mod class {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct ClassType<'arena>(pub Str<'arena>);

    impl_id!(ClassType, mangle = true, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> ClassType<'arena> {
            ClassType(Str::new_str(
                alloc,
                hhbc_by_ref_hhbc_string_utils::strip_global_ns(
                    &hhbc_by_ref_hhbc_string_utils::mangle(s.to_string()),
                ),
            ))
        }
    });

    impl<'arena> ClassType<'arena> {
        pub fn from_ast_name_and_mangle(
            alloc: &'arena bumpalo::Bump,
            s: impl std::convert::Into<std::string::String>,
        ) -> Self {
            use crate::Id;
            self::ClassType::<'arena>::from_ast_name(alloc, s.into().as_str())
        }
    }
}

pub mod prop {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct PropType<'arena>(pub Str<'arena>);

    impl_id!(PropType, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> PropType<'arena> {
            PropType(Str::new_str(
                alloc,
                hhbc_by_ref_hhbc_string_utils::strip_global_ns(s),
            ))
        }
    });
    impl_add_suffix!(PropType);
}

pub mod method {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct MethodType<'arena>(pub Str<'arena>);

    impl_id!(MethodType, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> MethodType<'arena> {
            MethodType(Str::new_str(
                alloc,
                hhbc_by_ref_hhbc_string_utils::strip_global_ns(s),
            ))
        }
    });
    impl_add_suffix!(MethodType);

    impl<'arena> MethodType<'arena> {
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
    pub struct FunctionType<'arena>(pub Str<'arena>);

    impl_id!(FunctionType, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> FunctionType<'arena> {
            FunctionType(Str::new_str(
                alloc,
                hhbc_by_ref_hhbc_string_utils::strip_global_ns(s),
            ))
        }
    });
    impl_add_suffix!(FunctionType);
}

// escape reserved keyword via r#
pub mod r#const {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct ConstType<'arena>(pub Str<'arena>);

    impl_id!(ConstType, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> ConstType<'arena> {
            ConstType(Str::new_str(
                alloc,
                hhbc_by_ref_hhbc_string_utils::strip_global_ns(s),
            ))
        }
    });
}

pub mod record {
    use ffi::Str;

    #[derive(Copy, Clone)]
    #[repr(C)]
    pub struct RecordType<'arena>(pub Str<'arena>);

    impl_id!(RecordType, mangle = false, {
        fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> RecordType<'arena> {
            RecordType(Str::new_str(
                alloc,
                hhbc_by_ref_hhbc_string_utils::strip_global_ns(s),
            ))
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
        let id = prop::PropType(ffi::Str::new_str(&alloc, "Some"));
        let id = prop::PropType::add_suffix(&alloc, &id, "Property");
        assert_eq!("SomeProperty", id.to_raw_string());
    }

    #[test]
    fn test_from_ast_name() {
        let alloc = bumpalo::Bump::new();
        let id = method::MethodType::from_ast_name(&alloc, "meth");
        assert_eq!("meth", id.to_raw_string());
    }
}

#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhbc_id<'a, 'arena>(
    _: class::ClassType<'arena>,
    _: function::FunctionType<'arena>,
    _: method::MethodType<'arena>,
    _: prop::PropType<'arena>,
    _: r#const::ConstType<'arena>,
    _: record::RecordType<'arena>,
) {
    unimplemented!()
}

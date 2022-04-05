// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//Note: `$type<'arena>` is a newtype over a `&'arena str`
// (`class::Type<'arena>`, `function::Type<'arena>`, ...). We
//intend that these types borrow strings stored in `InstrSeq`
//arenas.

//Note: We manually write these definitions "outside" `impl_id` macro
//so as to allow `cbindgen` to see the definitions in order
//that it may generate compatible C structs.

use bstr::BStr;
use ffi::Str;

macro_rules! impl_id {
    ($type: ident) => {
        impl<'arena> $type<'arena> {
            pub fn new(s: ffi::Str<'arena>) -> Self {
                Self(s)
            }

            pub fn unsafe_into_string(self) -> std::string::String {
                self.0.unsafe_as_str().into()
            }

            pub fn unsafe_as_str(&self) -> &'arena str {
                self.0.unsafe_as_str()
            }

            pub fn as_ffi_str(&self) -> ffi::Str<'arena> {
                self.0
            }

            pub fn as_bstr(&self) -> &'arena BStr {
                self.0.as_bstr()
            }
        }

        pub fn from_raw_string<'arena>(alloc: &'arena bumpalo::Bump, s: &str) -> $type<'arena> {
            $type(Str::new_str(alloc, s))
        }

        impl write_bytes::DisplayBytes for $type<'_> {
            fn fmt(&self, f: &mut write_bytes::BytesFormatter<'_>) -> std::io::Result<()> {
                self.0.fmt(f)
            }
        }

        impl<'arena> std::fmt::Debug for $type<'arena> {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(f, "{}({})", module_path!(), self.unsafe_as_str())
            }
        }
    };
}

macro_rules! impl_add_suffix {
    ($type: ident) => {
        fn from_raw_string_with_suffix<'arena>(
            alloc: &'arena bumpalo::Bump,
            s: &str,
            suffix: &str,
        ) -> $type<'arena> {
            let mut r = bumpalo::collections::String::<'arena>::with_capacity_in(
                s.len() + suffix.len(),
                alloc,
            );
            r.push_str(s);
            r.push_str(suffix);
            $type(ffi::Slice::new(r.into_bump_str().as_bytes()))
        }

        // ok (multiple impl Struct blocks are allowed if needed)
        impl<'arena> $type<'arena> {
            pub fn add_suffix(alloc: &'arena bumpalo::Bump, id: &Self, suffix: &str) -> Self {
                from_raw_string_with_suffix(alloc, id.0.unsafe_as_str(), suffix)
            }
        }
    };
}

pub mod class {
    use super::*;

    #[derive(Copy, Clone, PartialEq, Eq, Hash)]
    #[repr(C)]
    pub struct ClassType<'arena>(Str<'arena>);

    impl_id!(ClassType);

    impl<'arena> ClassType<'arena> {
        pub fn from_ast_name_and_mangle(
            alloc: &'arena bumpalo::Bump,
            s: impl std::convert::Into<std::string::String>,
        ) -> Self {
            ClassType(Str::new_str(
                alloc,
                hhbc_string_utils::strip_global_ns(&hhbc_string_utils::mangle(s.into())),
            ))
        }

        pub fn unsafe_to_unmangled_str(&self) -> std::borrow::Cow<'arena, str> {
            std::borrow::Cow::from(hhbc_string_utils::unmangle(self.unsafe_as_str().into()))
        }
    }
}

pub mod prop {
    use super::*;

    #[derive(Copy, Clone, PartialEq, Eq, Hash)]
    #[repr(C)]
    pub struct PropType<'arena>(Str<'arena>);

    impl_id!(PropType);
    impl_add_suffix!(PropType);

    impl<'arena> PropType<'arena> {
        pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> PropType<'arena> {
            PropType(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
        }
    }
}

pub mod method {
    use super::*;

    #[derive(Copy, Clone, PartialEq, Eq, Hash)]
    #[repr(C)]
    pub struct MethodType<'arena>(Str<'arena>);

    impl_id!(MethodType);
    impl_add_suffix!(MethodType);

    impl<'arena> MethodType<'arena> {
        pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> MethodType<'arena> {
            MethodType(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
        }

        pub fn from_ast_name_and_suffix(
            alloc: &'arena bumpalo::Bump,
            s: &str,
            suffix: &str,
        ) -> Self {
            from_raw_string_with_suffix(alloc, hhbc_string_utils::strip_global_ns(s), suffix)
        }
    }
}

pub mod function {
    use super::*;

    #[derive(Copy, Clone, PartialEq, Eq, Hash)]
    #[repr(C)]
    pub struct FunctionType<'arena>(Str<'arena>);

    impl_id!(FunctionType);
    impl_add_suffix!(FunctionType);

    impl<'arena> FunctionType<'arena> {
        pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> FunctionType<'arena> {
            FunctionType(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
        }
    }
}

pub mod constant {
    use super::*;

    #[derive(Copy, Clone, PartialEq, Eq, Hash)]
    #[repr(C)]
    pub struct ConstType<'arena>(Str<'arena>);

    impl_id!(ConstType);

    impl<'arena> ConstType<'arena> {
        pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> ConstType<'arena> {
            ConstType(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
        }
    }
}

pub mod record {
    use super::*;

    #[derive(Copy, Clone, PartialEq, Eq, Hash)]
    #[repr(C)]
    pub struct RecordType<'arena>(Str<'arena>);

    impl_id!(RecordType);

    impl<'arena> RecordType<'arena> {
        pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> RecordType<'arena> {
            RecordType(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_from_unsafe_as_str() {
        let alloc = bumpalo::Bump::new();
        assert_eq!("Foo", class::from_raw_string(&alloc, "Foo").unsafe_as_str());
    }

    #[test]
    fn test_add_suffix() {
        let alloc = bumpalo::Bump::new();
        let id = prop::PropType::new(ffi::Str::new_str(&alloc, "Some"));
        let id = prop::PropType::add_suffix(&alloc, &id, "Property");
        assert_eq!("SomeProperty", id.unsafe_as_str());
    }

    #[test]
    fn test_from_ast_name() {
        let alloc = bumpalo::Bump::new();
        let id = method::MethodType::from_ast_name(&alloc, "meth");
        assert_eq!("meth", id.unsafe_as_str());
    }
}

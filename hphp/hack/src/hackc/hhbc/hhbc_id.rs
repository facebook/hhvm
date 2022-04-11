// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Note: `$type<'arena>` is a newtype over a `&'arena str`
// (`class::Type<'arena>`, `function::Type<'arena>`, ...). We intend that these
// types borrow strings stored in `InstrSeq` arenas.

use bstr::{BStr, ByteSlice};
use ffi::Str;

macro_rules! impl_id {
    ($type: ident) => {
        impl<'arena> $type<'arena> {
            pub fn new(s: ffi::Str<'arena>) -> Self {
                Self(s)
            }

            pub fn is_empty(&self) -> bool {
                self.0.is_empty()
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

            pub fn from_raw_string(alloc: &'arena bumpalo::Bump, s: &str) -> $type<'arena> {
                $type(Str::new_str(alloc, s))
            }

            pub fn as_bytes(&self) -> &[u8] {
                self.0.as_bytes()
            }
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
        impl<'arena> $type<'arena> {
            fn from_raw_string_with_suffix(
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

            pub fn add_suffix(alloc: &'arena bumpalo::Bump, id: &Self, suffix: &str) -> Self {
                $type::from_raw_string_with_suffix(alloc, id.0.unsafe_as_str(), suffix)
            }
        }
    };
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[repr(C)]
pub struct ClassName<'arena>(Str<'arena>);

impl_id!(ClassName);

impl<'arena> ClassName<'arena> {
    pub fn from_ast_name_and_mangle(
        alloc: &'arena bumpalo::Bump,
        s: impl std::convert::Into<std::string::String>,
    ) -> Self {
        ClassName(Str::new_str(
            alloc,
            hhbc_string_utils::strip_global_ns(&hhbc_string_utils::mangle(s.into())),
        ))
    }

    pub fn from_ast_name_and_mangle_for_module(
        alloc: &'arena bumpalo::Bump,
        s: impl std::convert::Into<std::string::String>,
    ) -> Self {
        ClassName(Str::new_str(
            alloc,
            &*format!(
                "__module_{}",
                hhbc_string_utils::strip_global_ns(&hhbc_string_utils::mangle(s.into()))
            ),
        ))
    }

    pub fn unsafe_to_unmangled_str(&self) -> std::borrow::Cow<'arena, str> {
        std::borrow::Cow::from(hhbc_string_utils::unmangle(self.unsafe_as_str().into()))
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[repr(C)]
pub struct PropName<'arena>(Str<'arena>);

impl_id!(PropName);
impl_add_suffix!(PropName);

impl<'arena> PropName<'arena> {
    pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> PropName<'arena> {
        PropName(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[repr(C)]
pub struct MethodName<'arena>(Str<'arena>);

impl_id!(MethodName);
impl_add_suffix!(MethodName);

impl<'arena> MethodName<'arena> {
    pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> MethodName<'arena> {
        MethodName(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
    }

    pub fn from_ast_name_and_suffix(alloc: &'arena bumpalo::Bump, s: &str, suffix: &str) -> Self {
        MethodName::from_raw_string_with_suffix(
            alloc,
            hhbc_string_utils::strip_global_ns(s),
            suffix,
        )
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[repr(C)]
pub struct FunctionName<'arena>(Str<'arena>);

impl_id!(FunctionName);
impl_add_suffix!(FunctionName);

impl<'arena> FunctionName<'arena> {
    pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> FunctionName<'arena> {
        FunctionName(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[repr(C)]
pub struct ConstName<'arena>(Str<'arena>);

impl_id!(ConstName);

impl<'arena> ConstName<'arena> {
    pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> ConstName<'arena> {
        ConstName(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_from_unsafe_as_str() {
        let alloc = bumpalo::Bump::new();
        assert_eq!(
            "Foo",
            ClassName::from_raw_string(&alloc, "Foo").unsafe_as_str()
        );
    }

    #[test]
    fn test_add_suffix() {
        let alloc = bumpalo::Bump::new();
        let id = PropName::new(ffi::Str::new_str(&alloc, "Some"));
        let id = PropName::add_suffix(&alloc, &id, "Property");
        assert_eq!("SomeProperty", id.unsafe_as_str());
    }

    #[test]
    fn test_from_ast_name() {
        let alloc = bumpalo::Bump::new();
        let id = MethodName::from_ast_name(&alloc, "meth");
        assert_eq!("meth", id.unsafe_as_str());
    }
}

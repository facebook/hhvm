// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Note: `$type<'arena>` is a newtype over a `&'arena str`
// (`class::Type<'arena>`, `function::Type<'arena>`, ...). We intend that these
// types borrow strings stored in `InstrSeq` arenas.

use bstr::BStr;
use bstr::ByteSlice;
use ffi::Str;
use serde::Serialize;

macro_rules! impl_id {
    ($type: ident) => {
        impl<'arena> $type<'arena> {
            pub const fn new(s: ffi::Str<'arena>) -> Self {
                Self(s)
            }

            pub fn empty() -> Self {
                Self(ffi::Slice::new(b""))
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

            pub fn as_bytes(&self) -> &'arena [u8] {
                self.0.as_bstr().as_bytes()
            }

            pub fn from_bytes(alloc: &'arena bumpalo::Bump, s: &[u8]) -> $type<'arena> {
                $type(Str::new_slice(alloc, s))
            }

            pub fn from_raw_string(alloc: &'arena bumpalo::Bump, s: &str) -> $type<'arena> {
                $type(Str::new_str(alloc, s))
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

/// Conventionally this is "A_" followed by an integer
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd, Serialize)]
#[repr(C)]
pub struct AdataId {
    id: u32,
}

impl AdataId {
    pub const INVALID: Self = Self { id: u32::MAX };

    pub fn new(id: usize) -> Self {
        Self {
            id: id.try_into().unwrap(),
        }
    }

    pub fn parse(s: &str) -> Result<Self, std::num::ParseIntError> {
        use std::str::FromStr;
        Ok(Self {
            id: u32::from_str(s.strip_prefix("A_").unwrap_or(""))?,
        })
    }

    pub fn id(&self) -> u32 {
        self.id
    }
}

impl std::fmt::Display for AdataId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "A_{}", self.id)
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd, Serialize)]
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

    pub fn unsafe_to_unmangled_str(&self) -> std::borrow::Cow<'arena, str> {
        std::borrow::Cow::from(hhbc_string_utils::unmangle(self.unsafe_as_str().into()))
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd, Serialize)]
#[repr(C)]
pub struct PropName<'arena>(Str<'arena>);

impl_id!(PropName);

impl<'arena> PropName<'arena> {
    pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> PropName<'arena> {
        PropName(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd, Serialize)]
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

#[derive(Copy, Clone, Eq, Hash, Serialize)]
#[repr(C)]
pub struct FunctionName<'arena>(Str<'arena>);

impl_id!(FunctionName);
impl_add_suffix!(FunctionName);

impl<'arena> FunctionName<'arena> {
    pub fn from_ast_name(alloc: &'arena bumpalo::Bump, s: &str) -> FunctionName<'arena> {
        FunctionName(Str::new_str(alloc, hhbc_string_utils::strip_global_ns(s)))
    }
}

impl<'arena> PartialEq for FunctionName<'arena> {
    fn eq(&self, other: &Self) -> bool {
        self.as_bytes().eq_ignore_ascii_case(other.as_bytes())
    }
}

impl<'arena> Ord for FunctionName<'arena> {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        if self.eq(other) {
            return std::cmp::Ordering::Equal;
        }
        self.0.cmp(&other.0)
    }
}

impl<'arena> PartialOrd for FunctionName<'arena> {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd, Serialize)]
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
    use std::collections::BTreeSet;

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
        let id = FunctionName::new(ffi::Str::new_str(&alloc, "Some"));
        let id = FunctionName::add_suffix(&alloc, &id, "Func");
        assert_eq!("SomeFunc", id.unsafe_as_str());
    }

    #[test]
    fn test_from_ast_name() {
        let alloc = bumpalo::Bump::new();
        let id = MethodName::from_ast_name(&alloc, "meth");
        assert_eq!("meth", id.unsafe_as_str());
    }

    #[test]
    fn test_eq_function_name() {
        let alloc = bumpalo::Bump::new();
        let id1 = FunctionName::from_ast_name(&alloc, "foo2$memoize_impl");
        let id2 = FunctionName::from_ast_name(&alloc, "Foo2$memoize_impl");
        assert_eq!(id1, id2);
    }

    #[test]
    fn test_ord_function_name() {
        let alloc = bumpalo::Bump::new();
        let mut ids = BTreeSet::new();
        ids.insert(FunctionName::from_ast_name(&alloc, "foo"));
        ids.insert(FunctionName::from_ast_name(&alloc, "Foo"));
        ids.insert(FunctionName::from_ast_name(&alloc, "foo2"));
        ids.insert(FunctionName::from_ast_name(&alloc, "Bar"));
        ids.insert(FunctionName::from_ast_name(&alloc, "bar"));
        let expected = ["Bar", "foo", "foo2"];
        let ids: Vec<&str> = ids.into_iter().map(|id| id.unsafe_as_str()).collect();
        assert_eq!(expected, ids.as_slice());
    }
}

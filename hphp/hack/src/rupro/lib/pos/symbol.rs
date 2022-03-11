// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ToOxidized;
use eq_modulo_pos::EqModuloPos;
use indexmap::{IndexMap, IndexSet};
use intern::{
    string::{BytesId, StringId},
    BuildIdHasher,
};
use serde::{Deserialize, Serialize};
use std::collections::{HashMap, HashSet};

pub type BuildSymbolHasher = BuildIdHasher<u32>;
pub type SymbolMap<V> = HashMap<Symbol, V, BuildSymbolHasher>;
pub type SymbolSet = HashSet<Symbol, BuildSymbolHasher>;
pub type SymbolIndexMap<V> = IndexMap<Symbol, V, BuildSymbolHasher>;
pub type SymbolIndexSet = IndexSet<Symbol, BuildSymbolHasher>;

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct Symbol(pub StringId);
// nb: StringId implements Hash & Eq using the u32 id, and Ord
// using the underlying string after a fast check for equal ids.

impl Symbol {
    pub fn new<S: intern::string::IntoUtf8Bytes>(s: S) -> Self {
        Self(intern::string::intern(s))
    }
}

impl Symbol {
    pub fn as_str(&self) -> &str {
        self.0.as_str()
    }
}

impl std::ops::Deref for Symbol {
    type Target = str;

    fn deref(&self) -> &str {
        self.as_str()
    }
}

impl std::convert::AsRef<str> for Symbol {
    fn as_ref(&self) -> &str {
        self.as_str()
    }
}

impl std::fmt::Debug for Symbol {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self.as_str())
    }
}

impl std::fmt::Display for Symbol {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.as_str())
    }
}

impl From<&str> for Symbol {
    fn from(s: &str) -> Self {
        Self::new(s)
    }
}

impl EqModuloPos for Symbol {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        self == rhs
    }
}

impl<'a> ToOxidized<'a> for Symbol {
    type Output = &'a str;

    fn to_oxidized(&self, _arena: &'a bumpalo::Bump) -> &'a str {
        self.0.as_str()
    }
}

impl<'a, V: ToOxidized<'a>> ToOxidized<'a> for SymbolMap<V> {
    type Output = oxidized_by_ref::s_map::SMap<'a, V::Output>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        oxidized_by_ref::s_map::SMap::from(
            arena,
            self.iter()
                .map(|(k, v)| (k.to_oxidized(arena), v.to_oxidized(arena))),
        )
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct Bytes(pub BytesId);
// nb: BytesId implements Hash & Eq using the u32 id, and Ord
// using the underlying bytestring after a fast check for equal ids.

impl Bytes {
    pub fn new<S: AsRef<[u8]>>(s: S) -> Self {
        Self(intern::string::intern_bytes(s.as_ref()))
    }
}

impl Bytes {
    pub fn as_bytes(&self) -> &[u8] {
        self.0.as_bytes()
    }

    pub fn as_bstr(&self) -> &bstr::BStr {
        self.0.as_bytes().into()
    }
}

impl std::ops::Deref for Bytes {
    type Target = [u8];

    fn deref(&self) -> &[u8] {
        self.as_bytes()
    }
}

impl std::convert::AsRef<[u8]> for Bytes {
    fn as_ref(&self) -> &[u8] {
        self.as_bytes()
    }
}

impl std::fmt::Debug for Bytes {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self.as_bstr())
    }
}

impl std::fmt::Display for Bytes {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.as_bstr())
    }
}

impl From<&[u8]> for Bytes {
    fn from(s: &[u8]) -> Self {
        Self::new(s)
    }
}

impl From<&bstr::BStr> for Bytes {
    fn from(s: &bstr::BStr) -> Self {
        Self::new(s.as_ref())
    }
}

impl From<&str> for Bytes {
    fn from(s: &str) -> Self {
        Self::new(s)
    }
}

impl EqModuloPos for Bytes {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        self == rhs
    }
}

impl<'a> ToOxidized<'a> for Bytes {
    type Output = &'a [u8];

    fn to_oxidized(&self, _arena: &'a bumpalo::Bump) -> &'a [u8] {
        self.0.as_bytes()
    }
}

macro_rules! common_impls {
    ($name:ident) => {
        impl $name {
            pub fn new<S: intern::string::IntoUtf8Bytes>(s: S) -> Self {
                Self(Symbol::new(s))
            }

            pub fn as_str(&self) -> &str {
                self.0.as_str()
            }

            pub fn as_symbol(self) -> Symbol {
                self.0
            }
        }

        impl std::ops::Deref for $name {
            type Target = str;

            fn deref(&self) -> &str {
                self.as_str()
            }
        }

        impl std::convert::AsRef<str> for $name {
            fn as_ref(&self) -> &str {
                self.as_str()
            }
        }

        impl std::fmt::Debug for $name {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(f, "{:?}", self.as_str())
            }
        }

        impl std::fmt::Display for $name {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(f, "{}", self.as_str())
            }
        }

        impl From<&str> for $name {
            fn from(s: &str) -> Self {
                Self::new(s)
            }
        }

        impl<'a> ToOxidized<'a> for $name {
            type Output = &'a str;

            fn to_oxidized(&self, _arena: &'a bumpalo::Bump) -> &'a str {
                self.as_symbol().0.as_str()
            }
        }
    };
    ($name:ident, $map:ident) => {
        common_impls!($name);
        impl<'a, V: ToOxidized<'a>> ToOxidized<'a> for $map<V> {
            type Output = oxidized_by_ref::s_map::SMap<'a, V::Output>;

            fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
                oxidized_by_ref::s_map::SMap::from(
                    arena,
                    self.iter()
                        .map(|(k, v)| (k.to_oxidized(arena), v.to_oxidized(arena))),
                )
            }
        }
    };
    ($name:ident, $hashmap:ident, $indexmap:ident) => {
        common_impls!($name, $hashmap);
        impl<'a, V: ToOxidized<'a>> ToOxidized<'a> for $indexmap<V> {
            type Output = oxidized_by_ref::s_map::SMap<'a, V::Output>;

            fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
                oxidized_by_ref::s_map::SMap::from(
                    arena,
                    self.iter()
                        .map(|(k, v)| (k.to_oxidized(arena), v.to_oxidized(arena))),
                )
            }
        }
    };
}

// The following newtype wrappers are all for name categories that are
// disjoint from each other.
// Toplevel names can have namespace qualifiers, unlike member names.
// Toplevel names are not case sensitive in HHVM
//
// Any one of these name wrappers could turn into an enum if necessary
// to avoid stringly typed mangled names during compilation.

pub type BuildTypeNameHasher = BuildSymbolHasher;
pub type TypeNameMap<V> = HashMap<TypeName, V>;
pub type TypeNameSet = HashSet<TypeName>;
pub type TypeNameIndexMap<V> = IndexMap<TypeName, V>;
pub type TypeNameIndexSet = IndexSet<TypeName>;

/// A TypeName is the name of a class, interface, trait, type parameter,
/// type alias, newtype, or primitive type names like int, arraykey, etc.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct TypeName(pub Symbol);
common_impls!(TypeName, TypeNameMap, TypeNameIndexMap);

/// ModuleName is introduced by the experimental Modules feature and `internal`
/// visibility. ModuleNames are not bindable names and are not indended
/// to be interchangeable with any other kind of name.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct ModuleName(pub Symbol);
common_impls!(ModuleName);

/// Name of a top level constant.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct ConstName(pub Symbol);
common_impls!(ConstName);

/// Name of a top level function.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct FunName(pub Symbol);
common_impls!(FunName);

/// ClassConstName is the name of a class const, which are disjoint from
/// global constants, type constants, and other class members.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct ClassConstName(pub Symbol);
pub type BuildClassConstNameHasher = BuildSymbolHasher;
pub type ClassConstNameMap<V> = HashMap<ClassConstName, V>;
pub type ClassConstNameSet = HashSet<ClassConstName>;
pub type ClassConstNameIndexMap<V> = IndexMap<ClassConstName, V>;
pub type ClassConstNameIndexSet = IndexSet<ClassConstName>;
common_impls!(ClassConstName, ClassConstNameMap, ClassConstNameIndexMap);

#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct TypeConstName(pub Symbol);
pub type BuildTypeConstNameHasher = BuildSymbolHasher;
pub type TypeConstNameMap<V> = HashMap<TypeConstName, V>;
pub type TypeConstNameSet = HashSet<TypeConstName>;
pub type TypeConstNameIndexMap<V> = IndexMap<TypeConstName, V>;
pub type TypeConstNameIndexSet = IndexSet<TypeConstName>;
common_impls!(TypeConstName, TypeConstNameMap, TypeConstNameIndexMap);

#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct MethodName(pub Symbol);
pub type BuildMethodNameHasher = BuildSymbolHasher;
pub type MethodNameMap<V> = HashMap<MethodName, V>;
pub type MethodNameSet = HashSet<MethodName>;
pub type MethodNameIndexMap<V> = IndexMap<MethodName, V>;
pub type MethodNameIndexSet = IndexSet<MethodName>;
common_impls!(MethodName, MethodNameMap, MethodNameIndexMap);

#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct PropName(pub Symbol);
pub type BuildPropNameHasher = BuildSymbolHasher;
pub type PropNameMap<V> = HashMap<PropName, V>;
pub type PropNameSet = HashSet<PropName>;
pub type PropNameIndexMap<V> = IndexMap<PropName, V>;
pub type PropNameIndexSet = IndexSet<PropName>;
common_impls!(PropName, PropNameMap, PropNameIndexMap);

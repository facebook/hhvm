// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ToOxidized;
use intern::{string::StringId, BuildIdHasher};
use std::collections::{HashMap, HashSet};

pub type BuildSymbolHasher = BuildIdHasher<u32>;
pub type SymbolMap<V> = HashMap<Symbol, V, BuildSymbolHasher>;
pub type SymbolSet = HashSet<Symbol, BuildSymbolHasher>;

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
pub struct Symbol(pub StringId);
// nb: StringId implements Hash & Eq using the u32 id, and Ord
// using the underlying string after a fast check for equal ids.

impl Symbol {
    pub fn intern(s: &str) -> Self {
        Self(intern::string::intern(s))
    }
}

macro_rules! common_impls {
    ($name:ident) => {
        impl $name {
            pub fn as_str(&self) -> &str {
                self.0.as_str()
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

        impl<'a> ToOxidized<'a> for $name {
            type Output = &'a str;

            fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> &'a str {
                arena.alloc_str(self.0.as_str())
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
}

common_impls!(Symbol, SymbolMap);

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

/// A TypeName is the name of a class, interface, trait, type parameter,
/// type alias, newtype, or primitive type names like int, arraykey, etc.
#[derive(Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct TypeName(pub Symbol);
common_impls!(TypeName, TypeNameMap);

/// ModuleName is introduced by the experimental Modules feature and `internal`
/// visibility. ModuleNames are not bindable names and are not indended
/// to be interchangeable with any other kind of name.
#[derive(Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct ModuleName(pub Symbol);
common_impls!(ModuleName);

/// Name of a top level constant.
#[derive(Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct ConstName(pub Symbol);
common_impls!(ConstName);

/// Name of a top level function.
#[derive(Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct FunName(pub Symbol);
common_impls!(FunName);

/// ClassConstName is the name of a class const, which are disjoint from
/// global constants, type constants, and other class members.
#[derive(Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct ClassConstName(pub Symbol);
pub type BuildClassConstNameHasher = BuildSymbolHasher;
pub type ClassConstNameMap<V> = HashMap<ClassConstName, V>;
pub type ClassConstNameSet = HashSet<ClassConstName>;
common_impls!(ClassConstName, ClassConstNameMap);

#[derive(Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct TypeConstName(pub Symbol);
pub type BuildTypeConstNameHasher = BuildSymbolHasher;
pub type TypeConstNameMap<V> = HashMap<TypeConstName, V>;
pub type TypeConstNameSet = HashSet<TypeConstName>;
common_impls!(TypeConstName, TypeConstNameMap);

#[derive(Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct MethodName(pub Symbol);
pub type BuildMethodNameHasher = BuildSymbolHasher;
pub type MethodNameMap<V> = HashMap<MethodName, V>;
pub type MethodNameSet = HashSet<MethodName>;
common_impls!(MethodName, MethodNameMap);

#[derive(Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct PropName(pub Symbol);
pub type BuildPropNameHasher = BuildSymbolHasher;
pub type PropNameMap<V> = HashMap<PropName, V>;
pub type PropNameSet = HashSet<PropName>;
common_impls!(PropName, PropNameMap);

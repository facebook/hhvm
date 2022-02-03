// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use intern::{string::StringId, BuildIdHasher};
use std::collections::{HashMap, HashSet};

pub type BuildSymbolHasher = BuildIdHasher<u32>;
pub type SymbolMap<V> = HashMap<Symbol, V, BuildSymbolHasher>;
pub type SymbolSet = HashSet<Symbol, BuildSymbolHasher>;

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
pub struct Symbol(pub StringId);
// nb: StringId implements Hash & Eq using the u32 id, and Ord
// using the underlying string after a fast check for equal ids.

impl std::ops::Deref for Symbol {
    type Target = str;

    fn deref(&self) -> &str {
        self.0.as_str()
    }
}

impl std::convert::AsRef<str> for Symbol {
    fn as_ref(&self) -> &str {
        self.0.as_str()
    }
}

impl ToString for Symbol {
    fn to_string(&self) -> String {
        self.0.to_string()
    }
}

impl Symbol {
    pub fn intern(s: &str) -> Self {
        Self(intern::string::intern(s))
    }
}

pub type BuildTypeNameHasher = BuildSymbolHasher;
pub type TypeNameMap<V> = HashMap<TypeName, V>;
pub type TypeNameSet = HashSet<TypeName>;

/// A TypeName is the name of a class, interface, trait, type parameter,
/// type alias, newtype, or primitive type names like int, arraykey, etc.
///
/// This could become an enum at some point to distinguish plain user
/// classes from compiler-generated classes.
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct TypeName(pub Symbol);

impl ToString for TypeName {
    fn to_string(&self) -> String {
        self.0.to_string()
    }
}

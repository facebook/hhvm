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
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct TypeName(pub Symbol);

impl ToString for TypeName {
    fn to_string(&self) -> String {
        self.0.to_string()
    }
}

/// ModuleName is introduced by the experimental Modules feature and `internal`
/// visibility. ModuleNames are not bindable names and are not indended
/// to be interchangeable with any other kind of name.
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct ModuleName(pub Symbol);

/// Name of a top level constant.
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct ConstName(pub Symbol);

/// Name of a top level function.
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct FunName(pub Symbol);

/// ClassConstName is the name of a class const, which are disjoint from
/// global constants, type constants, and other class members.
#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct ClassConstName(pub Symbol);
pub type BuildClassConstNameHasher = BuildSymbolHasher;
pub type ClassConstNameMap<V> = HashMap<ClassConstName, V>;
pub type ClassConstNameSet = HashSet<ClassConstName>;

#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct TypeConstName(pub Symbol);

#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct MethodName(pub Symbol);
pub type BuildMethodNameHasher = BuildSymbolHasher;
pub type MethodNameMap<V> = HashMap<MethodName, V>;
pub type MethodNameSet = HashSet<MethodName>;

#[derive(Debug, Eq, PartialEq, Clone, Copy, Hash, Ord, PartialOrd)]
pub struct PropName(pub Symbol);
pub type BuildPropNameHasher = BuildSymbolHasher;
pub type PropNameMap<V> = HashMap<PropName, V>;
pub type PropNameSet = HashSet<PropName>;

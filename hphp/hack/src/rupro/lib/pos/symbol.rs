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

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub struct Symbol(pub StringId);

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

impl PartialOrd for Symbol {
    fn partial_cmp(&self, other: &Symbol) -> Option<std::cmp::Ordering> {
        self.0.as_str().partial_cmp(other.0.as_str())
    }
}
impl Ord for Symbol {
    fn cmp(&self, other: &Symbol) -> std::cmp::Ordering {
        self.0.as_str().cmp(other.0.as_str())
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

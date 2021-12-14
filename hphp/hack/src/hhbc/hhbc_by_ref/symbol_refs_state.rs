// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
use std::cmp::Ordering;
use std::collections::{BTreeMap, BTreeSet};

#[derive(Clone, Debug, Default)]
pub struct SymbolRefsState<'arena> {
    pub includes: IncludePathSet<'arena>,
    pub constants: SSet<'arena>,
    pub functions: SSet<'arena>,
    pub classes: SSet<'arena>,
}

impl<'arena> SymbolRefsState<'arena> {
    pub fn init(_alloc: &'arena bumpalo::Bump) -> Self {
        SymbolRefsState {
            includes: BTreeSet::new(),
            constants: BTreeSet::new(),
            functions: BTreeSet::new(),
            classes: BTreeSet::new(),
        }
    }
}

/// NOTE(hrust): order matters (hhbc_hhas write includes in sorted order)
pub type IncludePathSet<'arena> = BTreeSet<IncludePath<'arena>>;

pub type SSet<'arena> = BTreeSet<Str<'arena>>;

#[derive(Clone, Debug, Eq)]
#[repr(C)]
pub enum IncludePath<'arena> {
    Absolute(Str<'arena>),                         // /foo/bar/baz.php
    SearchPathRelative(Str<'arena>),               // foo/bar/baz.php
    IncludeRootRelative(Str<'arena>, Str<'arena>), // $_SERVER['PHP_ROOT'] . "foo/bar/baz.php"
    DocRootRelative(Str<'arena>),
}
impl<'arena> IncludePath<'arena> {
    pub fn into_doc_root_relative(
        self,
        alloc: &'arena bumpalo::Bump,
        include_roots: &BTreeMap<String, String>,
    ) -> IncludePath<'arena> {
        if let IncludePath::IncludeRootRelative(var, lit) = &self {
            use std::path::Path;
            match include_roots.get(var.unsafe_as_str()) {
                Some(prefix) => {
                    let path = Path::new(prefix).join(lit.unsafe_as_str());
                    let relative = path.is_relative();
                    let path_str = Str::new_str(alloc, path.to_str().expect("non UTF-8 path"));
                    return if relative {
                        IncludePath::DocRootRelative(path_str)
                    } else {
                        IncludePath::Absolute(path_str)
                    };
                }
                _ => return self, // This should probably never happen
            };
        }
        self
    }

    fn extract_str(&self) -> (&str, &str) {
        use IncludePath::*;
        match self {
            Absolute(s) | SearchPathRelative(s) | DocRootRelative(s) => (s.unsafe_as_str(), ""),
            IncludeRootRelative(s1, s2) => (s1.unsafe_as_str(), s2.unsafe_as_str()),
        }
    }
}
impl<'arena> Ord for IncludePath<'arena> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.extract_str().cmp(&other.extract_str())
    }
}
impl<'arena> PartialOrd for IncludePath<'arena> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl<'arena> PartialEq for IncludePath<'arena> {
    fn eq(&self, other: &Self) -> bool {
        self.extract_str().eq(&other.extract_str())
    }
}

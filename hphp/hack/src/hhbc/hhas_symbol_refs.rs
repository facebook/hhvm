// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::collections::{BTreeMap, BTreeSet};

/// Data structure for keeping track of symbols (and includes) we encounter in
///the course of emitting bytecode for an AST. We split them into these four
/// categories for the sake of HHVM, which has lookup function corresponding to each.
#[derive(Clone, Debug, Default)]
pub struct HhasSymbolRefs {
    pub includes: IncludePathSet,
    pub constants: SSet,
    pub functions: SSet,
    pub classes: SSet,
}

/// NOTE(hrust): order matters (hhbc_hhas write includes in sorted order)
pub type IncludePathSet = BTreeSet<IncludePath>;

type SSet = BTreeSet<String>;

#[derive(Clone, Debug, Eq)]
pub enum IncludePath {
    Absolute(String),                    // /foo/bar/baz.php
    SearchPathRelative(String),          // foo/bar/baz.php
    IncludeRootRelative(String, String), // $_SERVER['PHP_ROOT'] . "foo/bar/baz.php"
    DocRootRelative(String),
}
impl IncludePath {
    pub fn into_doc_root_relative(self, include_roots: &BTreeMap<String, String>) -> IncludePath {
        if let IncludePath::IncludeRootRelative(var, lit) = &self {
            use std::path::Path;
            match include_roots.get(var) {
                Some(prefix) => {
                    let path = Path::new(prefix).join(lit);
                    let relative = path.is_relative();
                    let path_str = path.to_str().expect("non UTF-8 path").to_owned();
                    return if relative {
                        IncludePath::DocRootRelative(path_str)
                    } else {
                        IncludePath::Absolute(path_str)
                    };
                }
                _ => panic!("missing var in include_roots: {}", var.to_string()),
            };
        }
        self
    }

    fn extract_str(&self) -> (&str, &str) {
        use IncludePath::*;
        match self {
            Absolute(s) | SearchPathRelative(s) | DocRootRelative(s) => (s, ""),
            IncludeRootRelative(s1, s2) => (s1, s2),
        }
    }
}
impl Ord for IncludePath {
    fn cmp(&self, other: &Self) -> Ordering {
        self.extract_str().cmp(&other.extract_str())
    }
}
impl PartialOrd for IncludePath {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl PartialEq for IncludePath {
    fn eq(&self, other: &Self) -> bool {
        self.extract_str().eq(&other.extract_str())
    }
}

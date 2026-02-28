// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cmp::Ordering;
use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::ffi::OsStr;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;

use bstr::BString;
use ffi::Vector;
use relative_path::RelativePath;
use serde::Serialize;

use crate::BytesId;
use crate::ClassName;
use crate::ConstName;
use crate::FunctionName;

/// Data structure for keeping track of symbols (and includes) we
/// encounter in the course of emitting bytecode for an AST. We split
/// them into these four categories for the sake of HHVM, which has
/// a dedicated lookup function corresponding to each.
#[derive(Default, Clone, Debug, Serialize)]
#[repr(C)]
pub struct SymbolRefs {
    pub includes: Vector<IncludePath>,
    pub constants: Vector<ConstName>,
    pub functions: Vector<FunctionName>,
    pub classes: Vector<ClassName>,
}

/// NOTE: order matters (hhbc_hhas write includes in sorted order)
pub type IncludePathSet = BTreeSet<IncludePath>;

#[derive(Clone, Debug, Eq, Serialize)]
#[repr(C)]
pub enum IncludePath {
    Absolute(BytesId),                     // /foo/bar/baz.php
    SearchPathRelative(BytesId),           // foo/bar/baz.php
    IncludeRootRelative(BytesId, BytesId), // $_SERVER['PHP_ROOT'] . "foo/bar/baz.php"
    DocRootRelative(BytesId),
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

impl IncludePath {
    pub fn resolve_include_roots(
        self,
        include_roots: &BTreeMap<BString, BString>,
        current_path: &RelativePath,
    ) -> IncludePath {
        match self {
            IncludePath::IncludeRootRelative(var, lit) => {
                match include_roots.get(var.as_bytes()) {
                    Some(prefix) => {
                        let path =
                            Path::new(OsStr::from_bytes(prefix)).join(OsStr::from_bytes(&lit));
                        let path_str = crate::intern_bytes(path.as_os_str().as_bytes());
                        if path.is_relative() {
                            IncludePath::DocRootRelative(path_str)
                        } else {
                            IncludePath::Absolute(path_str)
                        }
                    }
                    _ => self, // This should probably never happen
                }
            }
            IncludePath::SearchPathRelative(p) => {
                let pathbuf = current_path
                    .path()
                    .parent()
                    .unwrap_or_else(|| Path::new(""))
                    .join(OsStr::from_bytes(&p));
                let path_from_cur_dirname = crate::intern_bytes(pathbuf.as_os_str().as_bytes());
                IncludePath::SearchPathRelative(path_from_cur_dirname)
            }
            _ => self,
        }
    }

    pub fn extract_str(&self) -> (&[u8], &[u8]) {
        match self {
            IncludePath::Absolute(s)
            | IncludePath::SearchPathRelative(s)
            | IncludePath::DocRootRelative(s) => (s.as_bytes(), &[]),
            IncludePath::IncludeRootRelative(s1, s2) => (s1.as_bytes(), s2.as_bytes()),
        }
    }
}

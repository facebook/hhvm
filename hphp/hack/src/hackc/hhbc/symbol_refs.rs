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
use ffi::Str;
use ffi::Vector;
use relative_path::RelativePath;
use serde::Serialize;

use crate::ClassName;
use crate::ConstName;
use crate::FunctionName;

/// Data structure for keeping track of symbols (and includes) we
/// encounter in the course of emitting bytecode for an AST. We split
/// them into these four categories for the sake of HHVM, which has
/// a dedicated lookup function corresponding to each.
#[derive(Default, Clone, Debug, Serialize)]
#[repr(C)]
pub struct SymbolRefs<'arena> {
    pub includes: Vector<IncludePath<'arena>>,
    pub constants: Vector<ConstName>,
    pub functions: Vector<FunctionName<'arena>>,
    pub classes: Vector<ClassName<'arena>>,
}

/// NOTE(hrust): order matters (hhbc_hhas write includes in sorted order)
pub type IncludePathSet<'arena> = BTreeSet<IncludePath<'arena>>;

#[derive(Clone, Debug, Eq, Serialize)]
#[repr(C)]
pub enum IncludePath<'arena> {
    Absolute(Str<'arena>),                         // /foo/bar/baz.php
    SearchPathRelative(Str<'arena>),               // foo/bar/baz.php
    IncludeRootRelative(Str<'arena>, Str<'arena>), // $_SERVER['PHP_ROOT'] . "foo/bar/baz.php"
    DocRootRelative(Str<'arena>),
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

impl<'arena> IncludePath<'arena> {
    pub fn resolve_include_roots(
        self,
        alloc: &'arena bumpalo::Bump,
        include_roots: &BTreeMap<BString, BString>,
        current_path: &RelativePath,
    ) -> IncludePath<'arena> {
        match self {
            IncludePath::IncludeRootRelative(var, lit) => {
                match include_roots.get(var.as_bstr()) {
                    Some(prefix) => {
                        let path =
                            Path::new(OsStr::from_bytes(prefix)).join(OsStr::from_bytes(&lit));
                        let relative = path.is_relative();
                        let path_str = Str::new_str(alloc, path.to_str().expect("non UTF-8 path"));
                        return if relative {
                            IncludePath::DocRootRelative(path_str)
                        } else {
                            IncludePath::Absolute(path_str)
                        };
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
                let path_from_cur_dirname = Str::new_str(alloc, pathbuf.to_str().unwrap());
                IncludePath::SearchPathRelative(path_from_cur_dirname)
            }
            _ => self,
        }
    }

    pub fn extract_str(&self) -> (&str, &str) {
        match self {
            IncludePath::Absolute(s)
            | IncludePath::SearchPathRelative(s)
            | IncludePath::DocRootRelative(s) => (s.unsafe_as_str(), ""),
            IncludePath::IncludeRootRelative(s1, s2) => (s1.unsafe_as_str(), s2.unsafe_as_str()),
        }
    }
}

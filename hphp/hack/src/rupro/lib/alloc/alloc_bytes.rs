// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::pos::{Prefix, RelativePath, Symbol};
use crate::reason::Reason;

use super::{Allocator, GlobalAllocator};

impl GlobalAllocator {
    pub fn symbol(&self, symbol: &str) -> Symbol {
        self.bytes.mk_str(symbol)
    }

    pub fn concat<S1: AsRef<str>, S2: AsRef<str>>(&self, s1: S1, s2: S2) -> Symbol {
        let s1 = s1.as_ref();
        let s2 = s2.as_ref();
        self.symbol(&format!("{}{}", s1, s2))
    }

    pub fn relative_path(&self, prefix: Prefix, suffix: &std::path::Path) -> RelativePath {
        RelativePath::new(prefix, self.symbol(suffix.to_str().unwrap()))
    }

    pub fn relative_path_from_ast(
        &self,
        path: &oxidized::relative_path::RelativePath,
    ) -> RelativePath {
        self.relative_path(path.prefix(), path.path())
    }

    pub fn relative_path_from_decl(
        &self,
        path: &oxidized_by_ref::relative_path::RelativePath<'_>,
    ) -> RelativePath {
        self.relative_path(path.prefix(), path.path())
    }
}

impl<R: Reason> Allocator<R> {
    pub fn symbol(&self, symbol: &str) -> Symbol {
        self.global.symbol(symbol)
    }

    pub fn concat<S1: AsRef<str>, S2: AsRef<str>>(&self, s1: S1, s2: S2) -> Symbol {
        self.global.concat(s1, s2)
    }

    pub fn relative_path(&self, prefix: Prefix, suffix: &std::path::Path) -> RelativePath {
        self.global.relative_path(prefix, suffix)
    }

    pub fn relative_path_from_ast(
        &self,
        path: &oxidized::relative_path::RelativePath,
    ) -> RelativePath {
        self.global.relative_path_from_ast(path)
    }

    pub fn relative_path_from_decl(
        &self,
        path: &oxidized_by_ref::relative_path::RelativePath<'_>,
    ) -> RelativePath {
        self.global.relative_path_from_decl(path)
    }
}

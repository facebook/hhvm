// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

use crate::pos::Symbol;

pub use oxidized::relative_path::Prefix;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct RelativePath {
    prefix: Prefix,
    suffix: Symbol,
}

impl RelativePath {
    pub fn new(prefix: Prefix, suffix: Symbol) -> Self {
        Self { prefix, suffix }
    }

    pub fn to_absolute(&self, ctx: &RelativePathCtx) -> PathBuf {
        let prefix = match self.prefix {
            Prefix::Root => &ctx.root,
            Prefix::Hhi => &ctx.hhi,
            Prefix::Tmp => &ctx.tmp,
            Prefix::Dummy => &ctx.dummy,
        };
        prefix.join(&self.suffix)
    }
}

#[derive(Debug, Clone)]
pub struct RelativePathCtx {
    pub root: PathBuf,
    pub hhi: PathBuf,
    pub tmp: PathBuf,
    pub dummy: PathBuf,
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use intern::string::BytesId;
use std::ffi::OsStr;
use std::os::unix::ffi::OsStrExt;
use std::path::{Path, PathBuf};

pub use oxidized::relative_path::Prefix;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct RelativePath {
    prefix: Prefix,
    suffix: BytesId,
}

impl RelativePath {
    pub fn intern<P: AsRef<Path>>(prefix: Prefix, suffix: P) -> Self {
        let suffix = intern::string::intern_bytes(suffix.as_ref().as_os_str().as_bytes());
        Self { prefix, suffix }
    }

    pub fn to_absolute(&self, ctx: &RelativePathCtx) -> PathBuf {
        let mut buf = match self.prefix {
            Prefix::Root => &ctx.root,
            Prefix::Hhi => &ctx.hhi,
            Prefix::Tmp => &ctx.tmp,
            Prefix::Dummy => &ctx.dummy,
        }
        .to_owned();
        buf.push(&OsStr::from_bytes(self.suffix.as_bytes()));
        buf
    }

    pub fn to_oxidized(&self) -> oxidized::relative_path::RelativePath {
        oxidized::relative_path::RelativePath::make(
            self.prefix,
            OsStr::from_bytes(self.suffix.as_bytes()).into(),
        )
    }
}

#[derive(Debug, Clone)]
pub struct RelativePathCtx {
    pub root: PathBuf,
    pub hhi: PathBuf,
    pub tmp: PathBuf,
    pub dummy: PathBuf,
}

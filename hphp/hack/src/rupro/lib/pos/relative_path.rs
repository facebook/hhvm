// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use intern::string::BytesId;
use std::ffi::OsStr;
use std::fmt;
use std::os::unix::ffi::OsStrExt;
use std::path::{Path, PathBuf};

pub use oxidized::relative_path::Prefix;

#[derive(Clone, Copy, PartialEq, Eq, Hash)]
pub struct RelativePath {
    prefix: Prefix,
    suffix: BytesId,
}

impl RelativePath {
    pub fn new<P: AsRef<Path>>(prefix: Prefix, suffix: P) -> Self {
        let suffix = intern::string::intern_bytes(suffix.as_ref().as_os_str().as_bytes());
        Self::from_bytes_id(prefix, suffix)
    }

    pub fn from_bytes_id(prefix: Prefix, suffix: BytesId) -> Self {
        Self { prefix, suffix }
    }

    #[inline]
    pub fn prefix(&self) -> Prefix {
        self.prefix
    }

    #[inline]
    pub fn suffix(&self) -> BytesId {
        self.suffix
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

impl From<&oxidized::relative_path::RelativePath> for RelativePath {
    fn from(path: &oxidized::relative_path::RelativePath) -> Self {
        Self::new(path.prefix(), path.path())
    }
}

impl From<&oxidized_by_ref::relative_path::RelativePath<'_>> for RelativePath {
    fn from(path: &oxidized_by_ref::relative_path::RelativePath<'_>) -> Self {
        Self::new(path.prefix(), path.path())
    }
}

impl fmt::Debug for RelativePath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{}|{}",
            self.prefix,
            Path::new(OsStr::from_bytes(self.suffix.as_bytes())).display()
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

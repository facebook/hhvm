// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ToOxidized;
use intern::string::BytesId;
use ocamlrep::{FromOcamlRep, ToOcamlRep};
use std::ffi::OsStr;
use std::fmt;
use std::os::unix::ffi::OsStrExt;
use std::path::{Path, PathBuf};

pub use oxidized::relative_path::Prefix;

#[derive(Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord)]
pub struct RelativePath {
    prefix: Prefix,
    suffix: BytesId,
}

impl RelativePath {
    pub fn new<P: AsRef<Path>>(prefix: Prefix, suffix: P) -> Self {
        let suffix = intern::string::intern_bytes(suffix.as_ref().as_os_str().as_bytes());
        Self::from_bytes_id(prefix, suffix)
    }

    pub const fn empty() -> Self {
        Self {
            prefix: Prefix::Dummy,
            suffix: BytesId::EMPTY,
        }
    }

    pub fn is_empty(&self) -> bool {
        self.prefix == Prefix::Dummy && self.suffix == BytesId::EMPTY
    }

    pub const fn from_bytes_id(prefix: Prefix, suffix: BytesId) -> Self {
        Self { prefix, suffix }
    }

    #[inline]
    pub const fn prefix(&self) -> Prefix {
        self.prefix
    }

    #[inline]
    pub const fn suffix(&self) -> BytesId {
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
}

impl<'a> ToOxidized<'a> for RelativePath {
    type Output = &'a oxidized_by_ref::relative_path::RelativePath<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(oxidized_by_ref::relative_path::RelativePath::new(
            self.prefix,
            Path::new(OsStr::from_bytes(self.suffix.as_bytes())),
        ))
    }
}

impl ToOcamlRep for RelativePath {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        alloc.add(&(
            self.prefix,
            Path::new(OsStr::from_bytes(self.suffix.as_bytes())),
        ))
    }
}

impl FromOcamlRep for RelativePath {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let path = oxidized::relative_path::RelativePath::from_ocamlrep(value)?;
        Ok(Self::from(&path))
    }
}

impl From<RelativePath> for oxidized::relative_path::RelativePath {
    fn from(path: RelativePath) -> Self {
        Self::make(
            path.prefix,
            OsStr::from_bytes(path.suffix.as_bytes()).into(),
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

#[derive(Debug, Default, Clone)]
pub struct RelativePathCtx {
    pub root: PathBuf,
    pub hhi: PathBuf,
    pub tmp: PathBuf,
    pub dummy: PathBuf,
}

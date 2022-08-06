// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::OsStr;
use std::fmt;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;
use std::path::PathBuf;

use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
pub use oxidized::relative_path::Prefix;

use crate::Bytes;
use crate::ToOxidized;

#[derive(Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord)]
#[derive(serde::Serialize, serde::Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub struct RelativePath {
    prefix: Prefix,
    suffix: Bytes,
}

impl RelativePath {
    pub fn new<P: AsRef<Path>>(prefix: Prefix, suffix: P) -> Self {
        let suffix = Bytes::new(suffix.as_ref().as_os_str().as_bytes());
        Self::from_bytes(prefix, suffix)
    }

    pub const fn empty() -> Self {
        Self {
            prefix: Prefix::Dummy,
            suffix: Bytes::EMPTY,
        }
    }

    pub fn is_empty(&self) -> bool {
        self.prefix == Prefix::Dummy && self.suffix == Bytes::EMPTY
    }

    pub const fn from_bytes(prefix: Prefix, suffix: Bytes) -> Self {
        Self { prefix, suffix }
    }

    #[inline]
    pub const fn prefix(&self) -> Prefix {
        self.prefix
    }

    #[inline]
    pub const fn suffix(&self) -> Bytes {
        self.suffix
    }

    #[inline]
    pub fn is_hhi(&self) -> bool {
        self.prefix() == Prefix::Hhi
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

impl arena_trait::TrivialDrop for RelativePath {}

impl<'a> ToOxidized<'a> for RelativePath {
    type Output = &'a oxidized_by_ref::relative_path::RelativePath<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(oxidized_by_ref::relative_path::RelativePath::new(
            self.prefix,
            Path::new(OsStr::from_bytes(self.suffix.as_bytes())),
        ))
    }
}

impl<'a> FromOcamlRepIn<'a> for RelativePath {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _arena: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
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
#[derive(serde::Serialize, serde::Deserialize)]
pub struct RelativePathCtx {
    pub root: PathBuf,
    pub hhi: PathBuf,
    pub tmp: PathBuf,
    pub dummy: PathBuf,
}

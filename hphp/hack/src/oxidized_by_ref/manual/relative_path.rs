// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::{self, Debug, Display};
use std::path::{Path, PathBuf};

use bumpalo::Bump;
use serde::{Deserialize, Serialize};

use no_pos_hash::NoPosHash;
use ocamlrep::ToOcamlRep;

pub use oxidized::relative_path::{Prefix, PrefixPathMap};

// Named Relative_path.default in OCaml, but it really isn't a suitable default
// for most purposes.
pub const EMPTY: RelativePath<'_> = RelativePath {
    prefix: Prefix::Dummy,
    path: None,
};

#[derive(Copy, Clone, Eq, Hash, NoPosHash, Ord, PartialEq, PartialOrd)]
pub struct RelativePath<'a> {
    prefix: Prefix,
    // Invariant: if Some, the path has length > 0. The use of Option here is
    // solely to allow us to assign EMPTY above, since it is not (yet) possible
    // to construct a Path in a const context.
    path: Option<&'a Path>,
}

impl<'a> RelativePath<'a> {
    pub const fn empty() -> &'static RelativePath<'static> {
        &EMPTY
    }

    pub fn new(prefix: Prefix, path: &'a Path) -> Self {
        let path = if path == Path::new("") {
            None
        } else {
            Some(path)
        };
        Self { prefix, path }
    }

    pub fn make(prefix: Prefix, path: &'a str) -> Self {
        Self::new(prefix, Path::new(path))
    }

    pub fn is_empty(&self) -> bool {
        self.prefix == Prefix::Dummy && self.path.is_none()
    }

    pub fn has_extension(&self, s: impl AsRef<std::ffi::OsStr>) -> bool {
        self.path().extension() == Some(s.as_ref())
    }

    pub fn path(&self) -> &Path {
        match self.path {
            None => Path::new(""),
            Some(path) => path,
        }
    }

    pub fn path_str(&self) -> Option<&str> {
        self.path().to_str()
    }

    pub fn prefix(&self) -> Prefix {
        self.prefix
    }

    pub fn to_absolute(&self, prefix_map: &PrefixPathMap) -> PathBuf {
        let prefix = self.prefix.to_path(prefix_map);
        let mut r = PathBuf::from(prefix);
        r.push(self.path());
        r
    }

    pub fn to_oxidized(&self) -> oxidized::relative_path::RelativePath {
        oxidized::relative_path::RelativePath::make(self.prefix, self.path().to_owned())
    }

    pub fn from_oxidized_in(
        path: &oxidized::relative_path::RelativePath,
        arena: &'a Bump,
    ) -> &'a Self {
        let path_str =
            bumpalo::collections::String::from_str_in(path.path_str(), arena).into_bump_str();
        arena.alloc(Self::make(path.prefix(), path_str))
    }
}

impl Debug for RelativePath<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        // Use a format string rather than Formatter::debug_struct to prevent
        // adding line breaks (they don't help readability here).
        write!(f, "RelativePath({})", self)
    }
}

impl Display for RelativePath<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}|{}", self.prefix, self.path().display())
    }
}

impl arena_trait::TrivialDrop for RelativePath<'_> {}

impl ToOcamlRep for RelativePath<'_> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        alloc.add(&(self.prefix, self.path()))
    }
}

impl<'a> ocamlrep::FromOcamlRepIn<'a> for RelativePath<'a> {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        let (prefix, path) = <(Prefix, &'a Path)>::from_ocamlrep_in(value, alloc)?;
        Ok(Self::new(prefix, path))
    }
}

// This custom implementation of Serialize/Deserialize encodes the RelativePath
// as a string. This allows using it as a map key in serde_json.
impl Serialize for RelativePath<'_> {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        let path_str = self.path().to_str().ok_or(serde::ser::Error::custom(
            "path contains invalid UTF-8 characters",
        ))?;
        serializer.serialize_str(&format!("{}|{}", self.prefix, path_str))
    }
}

// See comment on impl of Serialize above.
impl<'de> Deserialize<'de> for RelativePath<'de> {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        struct Visitor;

        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = RelativePath<'de>;

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(formatter, "a string for RelativePath")
            }

            fn visit_borrowed_str<E>(self, value: &'de str) -> Result<RelativePath, E>
            where
                E: serde::de::Error,
            {
                let mut split = value.splitn(2, '|');
                let prefix_str = split.next();
                let path_str = split.next();
                assert!(split.next() == None, "splitn(2) should yield <=2 results");
                let prefix = match prefix_str {
                    Some("root") => Prefix::Root,
                    Some("hhi") => Prefix::Hhi,
                    Some("tmp") => Prefix::Tmp,
                    Some("") => Prefix::Dummy,
                    _ => {
                        return Err(E::invalid_value(
                            serde::de::Unexpected::Other(&format!(
                                "unknown relative_path::Prefix: {:?}",
                                value
                            )),
                            &self,
                        ));
                    }
                };
                let path = match path_str {
                    Some(path_str) => path_str,
                    None => {
                        return Err(E::invalid_value(
                            serde::de::Unexpected::Other(
                                "missing pipe or got empty string \
                                 when deserializing RelativePath",
                            ),
                            &self,
                        ));
                    }
                };
                Ok(RelativePath::make(prefix, path))
            }
        }

        deserializer.deserialize_str(Visitor)
    }
}

pub type Map<'a, T> = arena_collections::SortedAssocList<'a, RelativePath<'a>, T>;

pub mod map {
    pub use super::Map;
}

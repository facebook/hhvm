// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;
use std::fmt::Display;
use std::path::Path;
use std::path::PathBuf;

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
#[derive(Deserialize, Serialize)]
#[derive(EqModuloPos, FromOcamlRep, FromOcamlRepIn, ToOcamlRep, NoPosHash)]
#[repr(u8)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
}
impl arena_trait::TrivialDrop for Prefix {}

impl Prefix {
    pub fn is_hhi(self) -> bool {
        matches!(self, Prefix::Hhi)
    }
}

impl TryFrom<usize> for Prefix {
    type Error = String;

    fn try_from(prefix_raw: usize) -> Result<Self, String> {
        match prefix_raw {
            0 => Ok(Prefix::Root),
            1 => Ok(Prefix::Hhi),
            2 => Ok(Prefix::Dummy),
            3 => Ok(Prefix::Tmp),
            _ => Err(format!("prefix {} is not defined", prefix_raw)),
        }
    }
}

impl Display for Prefix {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        // NB: This encoding is used in the impl of Serialize and Deserialize
        // for RelativePath below.
        match self {
            Self::Root => write!(f, "root"),
            Self::Hhi => write!(f, "hhi"),
            Self::Tmp => write!(f, "tmp"),
            Self::Dummy => write!(f, ""),
        }
    }
}

#[derive(Clone, Eq, Hash, PartialEq)]
#[derive(EqModuloPos, NoPosHash)]
pub struct RelativePath {
    prefix: Prefix,
    /// Representation invariant: the empty path is always encoded as `None`.
    /// This allows us to construct `RelativePath` in `const` contexts
    /// (because `Path::new` is not a `const fn`).
    path: Option<PathBuf>,
}

impl RelativePath {
    pub fn make(prefix: Prefix, path: PathBuf) -> Self {
        Self {
            prefix,
            path: if path.as_os_str().is_empty() {
                None
            } else {
                Some(path)
            },
        }
    }

    pub const EMPTY: Self = Self {
        prefix: Prefix::Dummy,
        path: None,
    };

    pub fn is_empty(&self) -> bool {
        self == &Self::EMPTY
    }

    pub fn has_extension(&self, s: impl AsRef<Path>) -> bool {
        self.path().extension() == Some(s.as_ref().as_os_str())
    }

    /// The relative path compared to the prefix
    pub fn path(&self) -> &Path {
        self.path.as_deref().unwrap_or(Path::new(""))
    }

    /// The relative path compared to the prefix as a string
    pub fn path_str(&self) -> &str {
        self.path().to_str().unwrap()
    }

    pub fn prefix(&self) -> Prefix {
        self.prefix
    }

    pub fn is_hhi(&self) -> bool {
        self.prefix.is_hhi()
    }

    pub fn to_absolute(&self, ctx: &RelativePathCtx) -> PathBuf {
        ctx.prefix_path(self.prefix).join(self.path())
    }
}

impl Display for RelativePath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}|{}", self.prefix, self.path().display())
    }
}

impl std::fmt::Debug for RelativePath {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self}")
    }
}

// This custom impl of Ord treats the path suffix as raw bytes instead of a
// Path, so that they are ordered the same as in OCaml (i.e., `foo.bar` comes
// before `foo/bar` lexicographically, but Rust Paths consider `foo/bar` to come
// first because the `foo` component is shorter than `foo.bar`)
impl Ord for RelativePath {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.prefix
            .cmp(&other.prefix)
            .then(self.path().as_os_str().cmp(other.path().as_os_str()))
    }
}

impl PartialOrd for RelativePath {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

// This custom implementation of Serialize/Deserialize encodes the RelativePath
// as a string. This allows using it as a map key in serde_json.
impl Serialize for RelativePath {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        let path_str = (self.path().to_str())
            .ok_or_else(|| serde::ser::Error::custom("path contains invalid UTF-8 characters"))?;
        serializer.serialize_str(&format!("{}|{}", self.prefix, path_str))
    }
}

// See comment on impl of Serialize above.
impl<'de> Deserialize<'de> for RelativePath {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        struct Visitor;

        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = RelativePath;

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
                write!(formatter, "a string for RelativePath")
            }

            fn visit_str<E>(self, value: &str) -> Result<RelativePath, E>
            where
                E: serde::de::Error,
            {
                let mut split = value.splitn(2, '|');
                let prefix_str = split.next();
                let path_str = split.next();
                assert!(split.next().is_none(), "splitn(2) should yield <=2 results");
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
                    Some(path_str) if path_str.is_empty() => None,
                    Some(path_str) => Some(PathBuf::from(path_str)),
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
                Ok(RelativePath { prefix, path })
            }
        }

        deserializer.deserialize_str(Visitor)
    }
}

impl ToOcamlRep for RelativePath {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let mut block = alloc.block_with_size(2);
        alloc.set_field(&mut block, 0, alloc.add(&self.prefix));
        alloc.set_field(&mut block, 1, alloc.add(self.path()));
        block.build()
    }
}

impl FromOcamlRep for RelativePath {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_tuple(value, 2)?;
        let prefix = ocamlrep::from::field(block, 0)?;
        let path: PathBuf = ocamlrep::from::field(block, 1)?;
        Ok(Self::make(prefix, path))
    }
}

pub type Map<T> = std::collections::BTreeMap<RelativePath, T>;

pub mod map {
    pub use super::Map;
}

#[derive(Debug, Default, Clone)]
#[derive(serde::Serialize, serde::Deserialize)]
pub struct RelativePathCtx {
    pub root: PathBuf,
    pub hhi: PathBuf,
    pub tmp: PathBuf,
    pub dummy: PathBuf,
}

impl RelativePathCtx {
    pub fn prefix_path(&self, prefix: Prefix) -> &Path {
        match prefix {
            Prefix::Root => &self.root,
            Prefix::Hhi => &self.hhi,
            Prefix::Tmp => &self.tmp,
            Prefix::Dummy => &self.dummy,
        }
    }
}

#[cfg(test)]
mod tests {
    use pretty_assertions::assert_eq;

    use super::*;

    #[test]
    fn test_valid_usize_prefix() {
        let valid_prefix: usize = 2;
        assert_eq!(Ok(Prefix::Dummy), Prefix::try_from(valid_prefix));
    }

    #[test]
    fn test_invalid_usize_prefix() {
        let invalid_prefix: usize = 22;

        assert_eq!(
            Err("prefix 22 is not defined".to_string()),
            Prefix::try_from(invalid_prefix)
        )
    }
}

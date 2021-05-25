// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::TryFrom;
use std::fmt::{self, Display};
use std::path::{Path, PathBuf};

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRep, FromOcamlRepIn, ToOcamlRep};
use serde::{Deserialize, Serialize};

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    Hash,
    FromOcamlRep,
    FromOcamlRepIn,
    ToOcamlRep,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
}
impl arena_trait::TrivialDrop for Prefix {}

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
        use Prefix::*;
        // NB: This encoding is used in the impl of Serialize and Deserialize
        // for RelativePath below.
        match self {
            Root => write!(f, "root"),
            Hhi => write!(f, "hhi"),
            Tmp => write!(f, "tmp"),
            Dummy => write!(f, ""),
        }
    }
}

pub struct PrefixPathMap {
    root: PathBuf,
    hhi: PathBuf,
    tmp: PathBuf,
    dummy: PathBuf,
}

impl Default for PrefixPathMap {
    fn default() -> Self {
        Self {
            root: PathBuf::new(),
            hhi: PathBuf::new(),
            tmp: PathBuf::new(),
            dummy: PathBuf::new(),
        }
    }
}

impl Prefix {
    pub fn to_path(self, map: &PrefixPathMap) -> &Path {
        match self {
            Self::Root => &map.root,
            Self::Hhi => &map.hhi,
            Self::Tmp => &map.tmp,
            Self::Dummy => &map.dummy,
        }
    }
}

#[derive(
    Clone,
    Debug,
    Eq,
    EqModuloPos,
    Hash,
    FromOcamlRep,
    ToOcamlRep,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd
)]
pub struct RelativePath {
    prefix: Prefix,
    path: PathBuf,
}

impl RelativePath {
    pub fn make(prefix: Prefix, path: PathBuf) -> Self {
        Self { prefix, path }
    }

    pub fn is_empty(&self) -> bool {
        self.prefix == Prefix::Dummy && self.path.as_os_str().is_empty()
    }

    pub fn has_extension(&self, s: impl AsRef<Path>) -> bool {
        self.path.extension() == Some(s.as_ref().as_os_str())
    }

    pub fn path(&self) -> &Path {
        &self.path
    }

    pub fn path_str(&self) -> &str {
        self.path.to_str().unwrap()
    }

    pub fn prefix(&self) -> Prefix {
        self.prefix
    }

    pub fn to_absolute(&self) -> PathBuf {
        let prefix_map = PrefixPathMap::default();
        let prefix = self.prefix.to_path(&prefix_map);
        let mut r = PathBuf::from(prefix);
        r.push(self.path.as_path());
        r
    }
}

impl Display for RelativePath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}|{}", self.prefix, self.path.display())
    }
}

// This custom implementation of Serialize/Deserialize encodes the RelativePath
// as a string. This allows using it as a map key in serde_json.
impl Serialize for RelativePath {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        let path_str = self.path.to_str().ok_or(serde::ser::Error::custom(
            "path contains invalid UTF-8 characters",
        ))?;
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

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(formatter, "a string for RelativePath")
            }

            fn visit_str<E>(self, value: &str) -> Result<RelativePath, E>
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
                    Some(path_str) => PathBuf::from(path_str),
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

pub type Map<T> = std::collections::BTreeMap<RelativePath, T>;

pub mod map {
    pub use super::Map;
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

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

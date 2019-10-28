// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::OcamlRep;
use std::convert::TryFrom;
use std::fmt::{self, Display};
use std::path::{Path, PathBuf};

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
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
        use Prefix::*;
        match self {
            Root => write!(f, "root"),
            Hhi => write!(f, "hhi"),
            Tmp => write!(f, "tmp"),
            Dummy => write!(f, "dummy"),
        }
    }
}

#[derive(Clone, Debug, Eq, OcamlRep, PartialEq)]
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
}

impl Display for RelativePath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {}", self.prefix, self.path.display())
    }
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

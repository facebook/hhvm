// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ocamlpool_rust::utils::*;
use ocamlrep::rc::RcOc;
use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;
use std::convert::TryFrom;
use std::fmt::{Display, Formatter, Result};
use std::path::{Path, PathBuf};

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
}

impl TryFrom<usize> for Prefix {
    type Error = String;

    fn try_from(prefix_raw: usize) -> std::result::Result<Self, String> {
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
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        use Prefix::*;
        match self {
            Root => write!(f, "root"),
            Hhi => write!(f, "hhi"),
            Tmp => write!(f, "tmp"),
            Dummy => write!(f, "dummy"),
        }
    }
}

#[derive(Clone, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub struct RelativePath(RcOc<(Prefix, PathBuf)>);

impl RelativePath {
    // TODO(shiqicao): consider adding a FromOcamlvalue derive
    pub unsafe fn from_ocamlvalue(block: &ocaml::Value) -> RelativePath {
        let prefix = Prefix::try_from(usize_field(block, 0)).unwrap();
        let path = str_field(block, 1);
        RelativePath::make(prefix, PathBuf::from(path.as_str()))
    }

    pub fn make(prefix: Prefix, pathbuf: PathBuf) -> Self {
        RelativePath(RcOc::new((prefix, pathbuf)))
    }

    pub fn is_empty(&self) -> bool {
        self.prefix() == Prefix::Dummy && self.path_str().is_empty()
    }

    pub fn ends_with(&self, s: &str) -> bool {
        (self.0).1.to_str().unwrap().ends_with(s)
    }

    pub fn path(&self) -> &Path {
        &(self.0).1
    }

    pub fn path_str(&self) -> &str {
        (self.0).1.to_str().unwrap()
    }

    pub fn prefix(&self) -> Prefix {
        (self.0).0
    }
}

impl Display for RelativePath {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        write!(f, "{} {}", (self.0).0, (self.0).1.display())
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

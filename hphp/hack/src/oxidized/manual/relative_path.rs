// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlpool_rust::utils::*;
use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;
use std::fmt::{Display, Formatter, Result};
use std::path::PathBuf;
use std::rc::Rc;

#[derive(Clone, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
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
pub struct RelativePath(Rc<(Prefix, PathBuf)>);

impl RelativePath {
    // TODO(shiqicao): consider adding a FromOcamlvalue derive
    pub unsafe fn from_ocamlvalue(block: &ocaml::Value) -> RelativePath {
        let prefix_raw = usize_field(block, 0);
        let prefix = match prefix_raw {
            0 => Prefix::Root,
            1 => Prefix::Hhi,
            2 => Prefix::Dummy,
            3 => Prefix::Tmp,
            _ => panic!("prefix {} is not defined", prefix_raw.to_string()),
        };
        let path = str_field(block, 1);
        RelativePath::make(prefix, PathBuf::from(path.as_str()))
    }

    pub fn make(prefix: Prefix, pathbuf: PathBuf) -> Self {
        RelativePath(Rc::new((prefix, pathbuf)))
    }

    pub fn ends_with(&self, s: &str) -> bool {
        (self.0).1.to_str().unwrap().ends_with(s)
    }

    pub fn path_str(&self) -> &str {
        (self.0).1.to_str().unwrap()
    }
}

impl Display for RelativePath {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        write!(f, "{} {}", (self.0).0, (self.0).1.display())
    }
}

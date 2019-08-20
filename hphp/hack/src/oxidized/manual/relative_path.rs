// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::IntoOcamlRep;
use ocamlvalue_macro::Ocamlvalue;
use std::path::PathBuf;
use std::rc::Rc;

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
}

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub struct RelativePath(Rc<(Prefix, PathBuf)>);

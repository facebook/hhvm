// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::gen::ast_defs::Pstring;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    NoPosHash,
    FromOcamlRep,
    ToOcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct DocComment(pub Rc<Pstring>);

impl DocComment {
    pub fn new(ps: Pstring) -> Self {
        Self(Rc::new(ps))
    }
}

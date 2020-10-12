// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use crate::gen::ast_defs::Pstring;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use serde::{Deserialize, Serialize};

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

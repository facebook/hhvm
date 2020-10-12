// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::Serialize;

use crate::gen::ast_defs::Pstring;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

#[derive(
    Copy,
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct DocComment<'a>(pub &'a Pstring<'a>);

impl arena_trait::TrivialDrop for DocComment<'_> {}

impl<'a> DocComment<'a> {
    pub fn new(ps: &'a Pstring) -> Self {
        Self(ps)
    }
}

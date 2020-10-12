// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ccc6fec56a4df7dc6f9a9bd10c50fddc>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub type Fixmes = i_map::IMap<i_map::IMap<pos::Pos>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ScouredComments {
    pub comments: Vec<(pos::Pos, prim_defs::Comment)>,
    pub fixmes: Fixmes,
    pub misuses: Fixmes,
    pub error_pos: Vec<pos::Pos>,
}

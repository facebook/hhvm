// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<71b19155be7a9c0e8b3b78d227b8922d>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::i_map;
use crate::pos;
use crate::prim_defs;

pub type Fixmes = i_map::IMap<i_map::IMap<pos::Pos>>;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct ScouredComments {
    pub comments: Vec<(pos::Pos, prim_defs::Comment)>,
    pub fixmes: Fixmes,
    pub misuses: Fixmes,
    pub error_pos: Vec<pos::Pos>,
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<2653857c1d781bfc8088e2880c4e3a6d>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
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
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ScouredComments {
    pub comments: Vec<(pos::Pos, prim_defs::Comment)>,
    pub fixmes: Fixmes,
    pub misuses: Fixmes,
    pub error_pos: Vec<pos::Pos>,
}

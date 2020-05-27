// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<490dcc758e12adb968e354805efc8a15>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub type Fixmes = i_map::IMap<i_map::IMap<std::rc::Rc<pos::Pos>>>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ScouredComments {
    pub comments: Vec<(std::rc::Rc<pos::Pos>, prim_defs::Comment)>,
    pub fixmes: Fixmes,
    pub misuses: Fixmes,
    pub error_pos: Vec<std::rc::Rc<pos::Pos>>,
}

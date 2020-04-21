// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<870250bf3f55318d13860bfe68ae8ee0>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub type Fixmes<'a> = i_map::IMap<'a, i_map::IMap<'a, pos::Pos<'a>>>;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct ScouredComments<'a> {
    pub comments: &'a [(pos::Pos<'a>, prim_defs::Comment<'a>)],
    pub fixmes: Fixmes<'a>,
    pub misuses: Fixmes<'a>,
    pub error_pos: &'a [pos::Pos<'a>],
}

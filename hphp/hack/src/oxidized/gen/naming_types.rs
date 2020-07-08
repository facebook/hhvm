// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<269a3d863edc91ddd89f5f9481b9f4ab>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum KindOfType {
    TClass = 0,
    TTypedef = 1,
    TRecordDef = 2,
}
impl TrivialDrop for KindOfType {}

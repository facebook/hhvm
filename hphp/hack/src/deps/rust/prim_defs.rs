// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum Comment {
    CmtLine(String),
    CmtBlock(String),
}

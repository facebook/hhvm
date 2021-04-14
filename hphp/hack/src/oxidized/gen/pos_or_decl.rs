// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ebd136fb9378c4c6d773b9ee2f87a180>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::pos::map;

pub type PosOrDecl = pos::Pos;

/// The decl and file of a position.
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
pub struct Ctx {
    pub decl: Option<decl_reference::DeclReference>,
    pub file: relative_path::RelativePath,
}

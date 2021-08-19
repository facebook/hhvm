// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3cabc2f2d1e2c8252e84c94c5c173c99>>
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
#[repr(C)]
pub struct Ctx {
    pub decl: Option<decl_reference::DeclReference>,
    pub file: relative_path::RelativePath,
}

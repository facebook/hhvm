// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<fcec3f55ebf8ce10022c9048d94c34a1>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

pub use crate::pos::map;
#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
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

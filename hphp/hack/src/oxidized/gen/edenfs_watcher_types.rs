// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e05e3e2daf5c197a55ea9e529e2aab94>>
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
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum Changes {
    /// List is not guaranteed to be deduplicated
    FileChanges(Vec<String>),
    CommitTransition {
        from_commit: String,
        to_commit: String,
        /// List is not guaranteed to be deduplicated
        file_changes: Vec<String>,
    },
}

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
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum EdenfsWatcherError {
    EdenfsWatcherError(String),
    /// There may have been some changes that Eden has lost track of.
    /// (e.g., due to Eden restarting).
    /// The string is just a description of what happened.
    LostChanges(String),
    /// The root directory is not inside an EdenFS mount.
    NonEdenWWW,
}

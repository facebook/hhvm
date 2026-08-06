// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a374fac80631054be1dc1aa182322d00>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// [Future_timeout] is specific to this FFI. All other cases should be kept in
/// sync with [edenfs_saved_state::SavedStateError]. Error payloads cross the
/// FFI boundary as strings.
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
#[repr(C, u8)]
pub enum SavedStateError {
    #[rust_to_ocaml(name = "No_saved_state")]
    NoSavedState,
    #[rust_to_ocaml(name = "Query_error")]
    QueryError(String),
    #[rust_to_ocaml(name = "Other_error")]
    OtherError(String),
    #[rust_to_ocaml(name = "Future_timeout")]
    FutureTimeout,
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
#[repr(C)]
pub struct LookupArgs {
    pub timeout_secs: isize,
    pub project_name: String,
    pub database_shard_name: Option<String>,
    pub repo_path: String,
    pub mergebase_rev: String,
    pub project_metadata: Option<String>,
}

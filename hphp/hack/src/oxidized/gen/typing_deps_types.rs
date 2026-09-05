// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e9dfb4b94cc7374d6856ed951e5fa631>>
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
pub enum DepgraphLoadError {
    #[rust_to_ocaml(name = "Depgraph_not_found")]
    DepgraphNotFound(String),
    #[rust_to_ocaml(name = "Depgraph_open_error")]
    DepgraphOpenError(String),
    #[rust_to_ocaml(name = "Depgraph_invalid_mode")]
    DepgraphInvalidMode(String),
    #[rust_to_ocaml(name = "Depgraph_not_loaded")]
    DepgraphNotLoaded,
}

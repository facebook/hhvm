// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<f86f6ad6d8e8acb09859c3af750cd89b>>
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

/// Auxiliary type used for communicating map-reduce data across FFI boundaries.
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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct MapReduceFfi {
    #[rust_to_ocaml(attr = "yojson.option")]
    pub tast_hashes: Option<tast_hashes::TastHashes>,
    #[rust_to_ocaml(attr = "yojson.option")]
    pub type_counter: Option<type_counter::TypeCounter>,
    #[rust_to_ocaml(attr = "yojson.option")]
    pub reason_collector: Option<reason_collector::ReasonCollector>,
}

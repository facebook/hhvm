// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<752de5326e8e27b4fc1134fecaeb0834>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
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
pub enum TypingDepsMode {
    /// Legacy mode, with SQLite saved-state dependency graph
    SQLiteMode,
    /// Custom mode, with the new custom dependency graph format.
    /// The parameter is the path to the database.
    CustomMode(Option<String>),
    /// Mode to produce both the legacy SQLite saved-state dependency graph,
    /// and, along side it, the new custom 64-bit dependency graph.
    ///
    /// The first parameter is (optionally) a path to an existing custom 64-bit
    /// dependency graph. If it is present, only new edges will be written,
    /// of not, all edges will be written.
    SaveCustomMode {
        graph: Option<String>,
        new_edges_dir: String,
    },
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum HashMode {
    Hash32Bit,
    Hash64Bit,
}
impl TrivialDrop for HashMode {}
arena_deserializer::impl_deserialize_in_arena!(HashMode);

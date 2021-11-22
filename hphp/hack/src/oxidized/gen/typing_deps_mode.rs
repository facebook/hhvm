// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<eda91fc96670a2d02a4cc386f3a70aca>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// Which dependency graph format are we using?
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
pub enum TypingDepsMode {
    /// Keep track of newly discovered edges in an in-memory delta.
    ///
    /// Optionally, the in-memory delta is backed by a pre-computed
    /// dependency graph stored using a custom file format.
    CustomMode(Option<String>),
    /// Mode that writes newly discovered edges to binary files on disk
    /// (one file per disk). Those binary files can then be post-processed
    /// using a tool of choice.
    ///
    /// The first parameter is (optionally) a path to an existing custom 64-bit
    /// dependency graph. If it is present, only new edges will be written,
    /// of not, all edges will be written.
    SaveCustomMode {
        graph: Option<String>,
        new_edges_dir: String,
        human_readable_dep_map_dir: Option<String>,
    },
}

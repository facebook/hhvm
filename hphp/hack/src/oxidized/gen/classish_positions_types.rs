// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ed68765ce15494f23be0a36d7e64c7a6>>
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

/// A relative position specifier in a class. Can later be
/// evaluated to an actual position in a file.
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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C, u8)]
pub enum Pos<P> {
    /// Use the precomputed position.
    Precomputed(P),
    /// Position at the end of the class body. Will return a zero-length
    /// position just before the closing brace.
    #[rust_to_ocaml(name = "Classish_end_of_body")]
    ClassishEndOfBody(String),
    /// Position at the start of the class body. Will return a zero-length
    /// position just after the opening brace.
    #[rust_to_ocaml(name = "Classish_start_of_body")]
    ClassishStartOfBody(String),
    /// Position encompassing the full range of the closing brace of
    /// the class body.
    #[rust_to_ocaml(name = "Classish_closing_brace")]
    ClassishClosingBrace(String),
}

/// Positional information for a single class
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
#[rust_to_ocaml(prefix = "classish_")]
#[repr(C)]
pub struct ClassishPositions<P> {
    pub start_of_body: P,
    pub end_of_body: P,
    pub closing_brace: P,
}

/// Positional information for a collection of classes
pub type ClassishPositionsTypes<P> = s_map::SMap<ClassishPositions<P>>;

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<151623cae266dfed04f2a0b4afc458c6>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

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
#[rust_to_ocaml(attr = "deriving (eq, hash, show, ord)")]
#[repr(u8)]
pub enum Severity {
    Warning,
    Err,
}
impl TrivialDrop for Severity {}
arena_deserializer::impl_deserialize_in_arena!(Severity);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    PartialEq,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C)]
pub struct UserDiagnostic<PrimPos, Pos> {
    pub severity: Severity,
    pub code: isize,
    pub claim: message::Message<PrimPos>,
    pub reasons: Vec<message::Message<Pos>>,
    #[rust_to_ocaml(attr = "hash.ignore")]
    #[rust_to_ocaml(attr = "equal fun _ _ -> true")]
    #[rust_to_ocaml(attr = "compare fun _ _ -> 0")]
    pub explanation: explanation::Explanation<Pos>,
    #[rust_to_ocaml(attr = "hash.ignore")]
    pub quickfixes: Vec<quickfix::Quickfix<PrimPos>>,
    pub custom_msgs: Vec<String>,
    pub is_fixmed: bool,
    pub function_pos: Option<PrimPos>,
}

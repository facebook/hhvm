// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<22fa2c7b043d68792cecf9cffb0690f1>>
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum PattNameContext {
    #[rust_to_ocaml(name = "Any_name_context")]
    AnyNameContext,
    #[rust_to_ocaml(name = "One_of_name_context")]
    OneOfNameContext(Vec<name_context::NameContext>),
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum PattNamingError {
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Unbound_name")]
    UnboundName {
        name_context: PattNameContext,
        name: patt_name::PattName,
    },
    #[rust_to_ocaml(name = "Invalid_naming")]
    InvalidNaming {
        errs: Vec<validation_err::ValidationErr>,
        patt: Box<PattNamingError>,
    },
}

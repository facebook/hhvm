// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ab2566ea8098fe5bb7481103852d4968>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

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
pub enum PattError {
    Primary(Primary),
    #[rust_to_ocaml(prefix = "patt_")]
    Apply {
        cb: Callback,
        err: Box<PattError>,
    },
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Apply_reasons")]
    ApplyReasons {
        rsns_cb: ReasonsCallback,
        secondary: Secondary,
    },
    #[rust_to_ocaml(prefix = "patt_")]
    Or {
        fst: Box<PattError>,
        snd: Box<PattError>,
    },
    Invalid {
        errs: Vec<validation_err::ValidationErr>,
        patt: Box<PattError>,
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
#[rust_to_ocaml(and)]
#[repr(u8)]
pub enum Primary {
    #[rust_to_ocaml(name = "Any_prim")]
    AnyPrim,
}
impl TrivialDrop for Primary {}
arena_deserializer::impl_deserialize_in_arena!(Primary);

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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Secondary {
    #[rust_to_ocaml(name = "Of_error")]
    OfError(Box<Secondary>),
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Violated_constraint")]
    ViolatedConstraint {
        cstr: patt_string::PattString,
        ty_sub: patt_locl_ty::PattLoclTy,
        ty_sup: patt_locl_ty::PattLoclTy,
    },
    #[rust_to_ocaml(prefix = "patt_ty_")]
    #[rust_to_ocaml(name = "Subtyping_error")]
    SubtypingError {
        sub: patt_locl_ty::PattLoclTy,
        sup: patt_locl_ty::PattLoclTy,
    },
    #[rust_to_ocaml(name = "Any_snd")]
    AnySnd,
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
#[rust_to_ocaml(and)]
#[repr(u8)]
pub enum Callback {
    #[rust_to_ocaml(name = "Any_callback")]
    AnyCallback,
}
impl TrivialDrop for Callback {}
arena_deserializer::impl_deserialize_in_arena!(Callback);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "deriving (eq, show, yojson)")]
#[repr(u8)]
pub enum ReasonsCallback {
    #[rust_to_ocaml(name = "Any_reasons_callback")]
    AnyReasonsCallback,
}
impl TrivialDrop for ReasonsCallback {}
arena_deserializer::impl_deserialize_in_arena!(ReasonsCallback);

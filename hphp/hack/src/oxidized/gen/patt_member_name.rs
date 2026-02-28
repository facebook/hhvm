// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<203eb2f672b68effba3cac5341c4fe17>>
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
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
#[repr(C, u8)]
pub enum PattMemberName {
    #[rust_to_ocaml(name = "Member_name")]
    MemberName {
        patt_string: patt_string::PattString,
    },
    As {
        lbl: patt_var::PattVar,
        patt: Box<PattMemberName>,
    },
    Wildcard,
    Invalid {
        errs: Vec<validation_err::ValidationErr>,
        patt: Box<PattMemberName>,
    },
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<97714b4d47fd35c35874c7e9db506240>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
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
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum PattName<'a> {
    As {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        lbl: &'a patt_var::PattVar<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        patt: &'a PattName<'a>,
    },
    #[rust_to_ocaml(prefix = "patt_")]
    Name {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        namespace: Namespace<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        name: patt_string::PattString<'a>,
    },
    Wildcard,
    Invalid {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        errs: &'a [validation_err::ValidationErr<'a>],
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        patt: &'a PattName<'a>,
    },
}
impl<'a> TrivialDrop for PattName<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattName<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
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
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
#[repr(C, u8)]
pub enum Namespace<'a> {
    Root,
    Slash {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        prefix: &'a Namespace<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        elt: patt_string::PattString<'a>,
    },
}
impl<'a> TrivialDrop for Namespace<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Namespace<'arena>);

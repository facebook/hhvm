// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<5ea4a3a9e05c99a005db0f630aea246f>>
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

pub type ParserOptions = global_options::GlobalOptions;

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
pub struct FfiT(
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
);

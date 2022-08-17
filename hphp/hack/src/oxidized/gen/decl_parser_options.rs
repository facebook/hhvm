// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0b5dd4be28e395ff8bbe49092ffabb86>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use eq_modulo_pos::EqModuloPosAndReason;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
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
    EqModuloPosAndReason,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C)]
pub struct DeclParserOptions {
    pub auto_namespace_map: Vec<(String, String)>,
    pub disable_xhp_element_mangling: bool,
    pub interpret_soft_types_as_like_types: bool,
    pub allow_new_attribute_syntax: bool,
    pub enable_xhp_class_modifier: bool,
    pub everything_sdt: bool,
    pub global_inference: bool,
    pub gi_reinfer_types: Vec<String>,
    pub php5_compat_mode: bool,
    pub hhvm_compat_mode: bool,
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d2f49730a3d4151fdfb0df67bff9414f>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

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
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct FullFidelityParserEnv {
    pub hhvm_compat_mode: bool,
    pub php5_compat_mode: bool,
    pub codegen: bool,
    pub disable_lval_as_an_expression: bool,
    pub disable_nontoplevel_declarations: bool,
    pub mode: Option<file_info::Mode>,
    pub rust: bool,
    pub disable_legacy_soft_typehints: bool,
    pub allow_new_attribute_syntax: bool,
    pub disable_legacy_attribute_syntax: bool,
    pub leak_rust_tree: bool,
    pub enable_xhp_class_modifier: bool,
    pub disable_xhp_element_mangling: bool,
    pub disable_xhp_children_declarations: bool,
    pub disable_modes: bool,
}

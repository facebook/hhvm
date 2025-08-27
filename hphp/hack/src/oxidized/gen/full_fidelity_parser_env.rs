// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<10ab3ea10e81f886c9caa263f03ca892>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

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
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (show, sexp_of)")]
#[repr(C)]
pub struct FullFidelityParserEnv {
    pub hhvm_compat_mode: bool,
    pub php5_compat_mode: bool,
    pub codegen: bool,
    pub disable_lval_as_an_expression: bool,
    pub mode: Option<file_info::Mode>,
    pub rust: bool,
    pub disable_legacy_soft_typehints: bool,
    pub disable_legacy_attribute_syntax: bool,
    pub leak_rust_tree: bool,
    pub enable_xhp_class_modifier: bool,
    pub disable_xhp_element_mangling: bool,
    pub disable_xhp_children_declarations: bool,
    pub interpret_soft_types_as_like_types: bool,
    pub is_systemlib: bool,
}

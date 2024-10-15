// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<b921175330f5bc3f627a9a8dd090eb60>>
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
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[repr(C)]
pub struct ParserOptions {
    pub disable_lval_as_an_expression: bool,
    pub const_static_props: bool,
    pub const_default_func_args: bool,
    pub abstract_static_props: bool,
    pub disallow_func_ptrs_in_constants: bool,
    pub enable_xhp_class_modifier: bool,
    pub disable_xhp_element_mangling: bool,
    pub allow_unstable_features: bool,
    pub hhvm_compat_mode: bool,
    pub hhi_mode: bool,
    pub codegen: bool,
    pub disable_legacy_soft_typehints: bool,
    pub disable_legacy_attribute_syntax: bool,
    pub disable_xhp_children_declarations: bool,
    pub const_default_lambda_args: bool,
    pub interpret_soft_types_as_like_types: bool,
    pub is_systemlib: bool,
    pub disallow_static_constants_in_default_func_args: bool,
    pub auto_namespace_map: Vec<(String, String)>,
    pub everything_sdt: bool,
    pub keep_user_attributes: bool,
    pub stack_size: isize,
    pub deregister_php_stdlib: bool,
    pub union_intersection_type_hints: bool,
    pub unwrap_concurrent: bool,
    pub disallow_silence: bool,
    pub no_parser_readonly_check: bool,
    pub parser_errors_only: bool,
    pub disable_hh_ignore_error: isize,
    pub allowed_decl_fixme_codes: i_set::ISet,
    pub use_legacy_experimental_feature_config: bool,
    pub experimental_features: s_map::SMap<experimental_features::FeatureStatus>,
    pub consider_unspecified_experimental_features_released: bool,
    pub package_v2: bool,
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
    pub s_map::SMap<experimental_features::FeatureStatus>,
    pub bool,
);

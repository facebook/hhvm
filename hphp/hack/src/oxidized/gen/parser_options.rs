// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7d2f8da3b6f5ca466c73f8bc286ce282>>
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
    pub hhvm_compat_mode: bool,
    pub hhi_mode: bool,
    pub codegen: bool,
    pub disable_lval_as_an_expression: bool,
    pub disable_legacy_soft_typehints: bool,
    pub const_static_props: bool,
    pub disable_legacy_attribute_syntax: bool,
    pub const_default_func_args: bool,
    pub abstract_static_props: bool,
    pub disallow_func_ptrs_in_constants: bool,
    pub enable_xhp_class_modifier: bool,
    pub disable_xhp_element_mangling: bool,
    pub disable_xhp_children_declarations: bool,
    pub const_default_lambda_args: bool,
    pub allow_unstable_features: bool,
    pub interpret_soft_types_as_like_types: bool,
    pub is_systemlib: bool,
    pub disallow_static_constants_in_default_func_args: bool,
    pub disallow_direct_superglobals_refs: bool,
    pub auto_namespace_map: Vec<(String, String)>,
    pub everything_sdt: bool,
    pub keep_user_attributes: bool,
    pub stack_size: isize,
    pub deregister_php_stdlib: bool,
    pub enable_class_level_where_clauses: bool,
    pub union_intersection_type_hints: bool,
    pub unwrap_concurrent: bool,
    pub disallow_silence: bool,
    pub no_parser_readonly_check: bool,
    pub parser_errors_only: bool,
    pub nameof_precedence: bool,
    pub disable_hh_ignore_error: isize,
    pub allowed_decl_fixme_codes: i_set::ISet,
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
);

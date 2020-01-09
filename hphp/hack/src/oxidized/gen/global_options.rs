// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<87a6bccc05e563761cef92454f5f7570>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

use crate::i_set;
use crate::infer_missing;
use crate::s_map;
use crate::s_set;

#[derive(Clone, Debug, OcamlRep)]
pub struct GlobalOptions {
    pub tco_experimental_features: s_set::SSet,
    pub tco_migration_flags: s_set::SSet,
    pub tco_dynamic_view: bool,
    pub tco_defer_class_declaration_threshold: Option<isize>,
    pub tco_max_times_to_defer_type_checking: Option<isize>,
    pub tco_prefetch_deferred_files: bool,
    pub tco_remote_type_check_threshold: Option<isize>,
    pub tco_remote_type_check: bool,
    pub tco_remote_worker_key: Option<String>,
    pub tco_remote_check_id: Option<String>,
    pub tco_remote_max_batch_size: isize,
    pub tco_remote_min_batch_size: isize,
    pub tco_num_remote_workers: isize,
    pub so_remote_version_specifier: Option<String>,
    pub so_remote_worker_vfs_checkout_threshold: isize,
    pub so_naming_sqlite_path: Option<String>,
    pub po_auto_namespace_map: Vec<(String, String)>,
    pub po_codegen: bool,
    pub po_deregister_php_stdlib: bool,
    pub po_disallow_execution_operator: bool,
    pub po_disallow_toplevel_requires: bool,
    pub po_disable_nontoplevel_declarations: bool,
    pub po_disable_static_closures: bool,
    pub po_allow_goto: bool,
    pub tco_log_inference_constraints: bool,
    pub tco_disallow_ambiguous_lambda: bool,
    pub tco_disallow_array_typehint: bool,
    pub tco_disallow_array_literal: bool,
    pub tco_language_feature_logging: bool,
    pub tco_unsafe_rx: bool,
    pub tco_disallow_scrutinee_case_value_type_mismatch: bool,
    pub tco_new_inference_lambda: bool,
    pub tco_timeout: isize,
    pub tco_disallow_invalid_arraykey: bool,
    pub tco_disallow_byref_dynamic_calls: bool,
    pub tco_disallow_byref_calls: bool,
    pub ignored_fixme_codes: i_set::ISet,
    pub ignored_fixme_regex: Option<String>,
    pub log_levels: s_map::SMap<isize>,
    pub po_disable_lval_as_an_expression: bool,
    pub tco_shallow_class_decl: bool,
    pub po_rust_parser_errors: bool,
    pub po_rust_lowerer: bool,
    pub profile_type_check_duration_threshold: f64,
    pub profile_type_check_twice: bool,
    pub profile_owner: String,
    pub profile_desc: String,
    pub tco_like_type_hints: bool,
    pub tco_union_intersection_type_hints: bool,
    pub tco_like_casts: bool,
    pub tco_simple_pessimize: f64,
    pub tco_complex_coercion: bool,
    pub tco_disable_partially_abstract_typeconsts: bool,
    pub error_codes_treated_strictly: i_set::ISet,
    pub tco_check_xhp_attribute: bool,
    pub tco_disallow_unresolved_type_variables: bool,
    pub tco_disallow_invalid_arraykey_constraint: bool,
    pub po_enable_class_level_where_clauses: bool,
    pub po_disable_legacy_soft_typehints: bool,
    pub po_disallowed_decl_fixmes: i_set::ISet,
    pub po_allow_new_attribute_syntax: bool,
    pub tco_infer_missing: infer_missing::InferMissing,
    pub tco_const_static_props: bool,
    pub po_disable_legacy_attribute_syntax: bool,
    pub tco_const_attribute: bool,
    pub po_const_default_func_args: bool,
    pub po_disallow_silence: bool,
    pub po_abstract_static_props: bool,
    pub po_disable_unset_class_const: bool,
    pub po_parser_errors_only: bool,
    pub tco_check_attribute_locations: bool,
    pub glean_service: String,
    pub glean_hostname: String,
    pub glean_port: isize,
    pub glean_reponame: String,
    pub po_disallow_func_ptrs_in_constants: bool,
    pub tco_error_php_lambdas: bool,
    pub tco_disallow_discarded_nullable_awaitables: bool,
    pub po_enable_xhp_class_modifier: bool,
}

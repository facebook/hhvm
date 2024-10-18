(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = {
  (* These options are set in both hhvm and hh config *)
  disable_lval_as_an_expression: bool;
  const_static_props: bool;
  const_default_func_args: bool;
  abstract_static_props: bool;
  disallow_func_ptrs_in_constants: bool;
  enable_xhp_class_modifier: bool;
  disable_xhp_element_mangling: bool;
  allow_unstable_features: bool;
  (* These options are set in hack config, but use the defaults in (from parser_options_impl.rs) hhvm*)
  hhvm_compat_mode: bool;
  hhi_mode: bool;
  codegen: bool;
  disable_legacy_soft_typehints: bool;
  disable_legacy_attribute_syntax: bool;
  disable_xhp_children_declarations: bool;
  const_default_lambda_args: bool;
  interpret_soft_types_as_like_types: bool;
  is_systemlib: bool;
  disallow_static_constants_in_default_func_args: bool;
  auto_namespace_map: (string * string) list;
  everything_sdt: bool;
  keep_user_attributes: bool;
  stack_size: int;
  deregister_php_stdlib: bool;
  union_intersection_type_hints: bool;
  unwrap_concurrent: bool;
  disallow_silence: bool;
  no_parser_readonly_check: bool;
  disable_hh_ignore_error: int;
  allowed_decl_fixme_codes: ISet.t;
  use_legacy_experimental_feature_config: bool;
  experimental_features: Experimental_features.feature_status SMap.t;
  consider_unspecified_experimental_features_released: bool;
  package_v2: bool;
}
[@@deriving show, eq]

let default =
  {
    disable_lval_as_an_expression = true (* false in rust *);
    const_static_props = false;
    const_default_func_args = false;
    abstract_static_props = false;
    disallow_func_ptrs_in_constants = false;
    enable_xhp_class_modifier = false;
    disable_xhp_element_mangling = false;
    allow_unstable_features = false;
    hhvm_compat_mode = false;
    hhi_mode = false;
    codegen = false;
    disable_legacy_soft_typehints = true;
    disable_legacy_attribute_syntax = false;
    disable_xhp_children_declarations = false;
    const_default_lambda_args = false;
    interpret_soft_types_as_like_types = false;
    is_systemlib = false;
    disallow_static_constants_in_default_func_args = false;
    auto_namespace_map = [];
    everything_sdt = false;
    deregister_php_stdlib = false;
    stack_size = 32 * 1024 * 1024;
    keep_user_attributes = false;
    union_intersection_type_hints = false;
    unwrap_concurrent = false;
    disallow_silence = false;
    no_parser_readonly_check = false;
    disable_hh_ignore_error = 0;
    allowed_decl_fixme_codes = ISet.empty;
    use_legacy_experimental_feature_config = true;
    experimental_features = SMap.empty;
    consider_unspecified_experimental_features_released = true;
    package_v2 = false;
  }

(* Changes here need to be synchronized with rust_parser_errors_ffi.rs *)
type ffi_t =
  bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * Experimental_features.feature_status SMap.t
  * bool

let to_rust_ffi_t po =
  ( po.hhvm_compat_mode,
    po.hhi_mode,
    po.codegen,
    po.disable_lval_as_an_expression,
    po.disable_legacy_soft_typehints,
    po.const_static_props,
    po.disable_legacy_attribute_syntax,
    po.const_default_func_args,
    po.abstract_static_props,
    po.disallow_func_ptrs_in_constants,
    po.enable_xhp_class_modifier,
    po.disable_xhp_element_mangling,
    po.disable_xhp_children_declarations,
    po.const_default_lambda_args,
    po.allow_unstable_features,
    po.interpret_soft_types_as_like_types,
    po.is_systemlib,
    po.disallow_static_constants_in_default_func_args,
    po.use_legacy_experimental_feature_config,
    po.experimental_features,
    po.consider_unspecified_experimental_features_released )

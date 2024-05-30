(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = {
  hhvm_compat_mode: bool;
  hhi_mode: bool;
  codegen: bool;
  disable_lval_as_an_expression: bool;
  disable_legacy_soft_typehints: bool;
  const_static_props: bool;
  disable_legacy_attribute_syntax: bool;
  const_default_func_args: bool;
  abstract_static_props: bool;
  disallow_func_ptrs_in_constants: bool;
  enable_xhp_class_modifier: bool;
  disable_xhp_element_mangling: bool;
  disable_xhp_children_declarations: bool;
  const_default_lambda_args: bool;
  allow_unstable_features: bool;
  interpret_soft_types_as_like_types: bool;
  is_systemlib: bool;
  disallow_static_constants_in_default_func_args: bool;
  disallow_direct_superglobals_refs: bool;
  auto_namespace_map: (string * string) list;
  everything_sdt: bool;
  keep_user_attributes: bool;
  stack_size: int;
  deregister_php_stdlib: bool;
  enable_class_level_where_clauses: bool;
  union_intersection_type_hints: bool;
  unwrap_concurrent: bool;
  disallow_silence: bool;
  no_parser_readonly_check: bool;
  parser_errors_only: bool;
  disable_hh_ignore_error: int;
  allowed_decl_fixme_codes: ISet.t;
}
[@@deriving show, eq]

let default =
  {
    hhvm_compat_mode = false;
    hhi_mode = false;
    codegen = false;
    disable_lval_as_an_expression = true;
    disable_legacy_soft_typehints = true;
    const_static_props = false;
    disable_legacy_attribute_syntax = false;
    const_default_func_args = false;
    abstract_static_props = false;
    disallow_func_ptrs_in_constants = false;
    enable_xhp_class_modifier = true;
    disable_xhp_element_mangling = true;
    disable_xhp_children_declarations = true;
    const_default_lambda_args = false;
    allow_unstable_features = false;
    interpret_soft_types_as_like_types = false;
    is_systemlib = false;
    disallow_static_constants_in_default_func_args = false;
    disallow_direct_superglobals_refs = false;
    auto_namespace_map = [];
    everything_sdt = false;
    keep_user_attributes = false;
    stack_size = 32 * 1024 * 1024;
    deregister_php_stdlib = false;
    enable_class_level_where_clauses = false;
    union_intersection_type_hints = false;
    unwrap_concurrent = false;
    disallow_silence = false;
    no_parser_readonly_check = false;
    parser_errors_only = false;
    disable_hh_ignore_error = 0;
    allowed_decl_fixme_codes = ISet.empty;
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
    po.disallow_direct_superglobals_refs )

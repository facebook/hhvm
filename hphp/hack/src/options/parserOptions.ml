(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

type t = GlobalOptions.t [@@deriving show]

let auto_namespace_map po = po.GlobalOptions.po_auto_namespace_map

let with_auto_namespace_map po m =
  { po with GlobalOptions.po_auto_namespace_map = m }

let codegen po = po.GlobalOptions.po_codegen

let with_codegen po b = { po with GlobalOptions.po_codegen = b }

let deregister_php_stdlib po = po.GlobalOptions.po_deregister_php_stdlib

let disallow_toplevel_requires po =
  po.GlobalOptions.po_disallow_toplevel_requires

let default = GlobalOptions.default

let const_default_func_args po = po.GlobalOptions.po_const_default_func_args

let with_const_default_func_args po b =
  { po with GlobalOptions.po_const_default_func_args = b }

let const_default_lambda_args po = po.GlobalOptions.po_const_default_lambda_args

let with_const_default_lambda_args po b =
  { po with GlobalOptions.po_const_default_lambda_args = b }

let with_disable_lval_as_an_expression po b =
  { po with GlobalOptions.po_disable_lval_as_an_expression = b }

let disable_lval_as_an_expression po =
  po.GlobalOptions.po_disable_lval_as_an_expression

let enable_class_level_where_clauses po =
  po.GlobalOptions.po_enable_class_level_where_clauses

let disable_legacy_soft_typehints po =
  po.GlobalOptions.po_disable_legacy_soft_typehints

let with_disable_legacy_soft_typehints po b =
  { po with GlobalOptions.po_disable_legacy_soft_typehints = b }

let allowed_decl_fixme_codes po = po.GlobalOptions.po_allowed_decl_fixme_codes

let disable_legacy_attribute_syntax po =
  po.GlobalOptions.po_disable_legacy_attribute_syntax

let with_disable_legacy_attribute_syntax po b =
  { po with GlobalOptions.po_disable_legacy_attribute_syntax = b }

let disallow_silence po = po.GlobalOptions.po_disallow_silence

let const_static_props po = po.GlobalOptions.tco_const_static_props

let with_const_static_props po b =
  { po with GlobalOptions.tco_const_static_props = b }

let abstract_static_props po = po.GlobalOptions.po_abstract_static_props

let with_abstract_static_props po b =
  { po with GlobalOptions.po_abstract_static_props = b }

let parser_errors_only po = po.GlobalOptions.po_parser_errors_only

let with_parser_errors_only po b =
  { po with GlobalOptions.po_parser_errors_only = b }

let disallow_func_ptrs_in_constants po =
  po.GlobalOptions.po_disallow_func_ptrs_in_constants

let with_disallow_func_ptrs_in_constants po b =
  { po with GlobalOptions.po_disallow_func_ptrs_in_constants = b }

let disable_xhp_element_mangling po =
  po.GlobalOptions.po_disable_xhp_element_mangling

let with_disable_xhp_element_mangling po b =
  { po with GlobalOptions.po_disable_xhp_element_mangling = b }

let with_keep_user_attributes po b =
  { po with GlobalOptions.po_keep_user_attributes = b }

let allow_unstable_features po = po.GlobalOptions.po_allow_unstable_features

let with_allow_unstable_features po b =
  { po with GlobalOptions.po_allow_unstable_features = b }

let disable_xhp_children_declarations po =
  po.GlobalOptions.po_disable_xhp_children_declarations

let with_disable_xhp_children_declarations po b =
  { po with GlobalOptions.po_disable_xhp_children_declarations = b }

let enable_xhp_class_modifier po = po.GlobalOptions.po_enable_xhp_class_modifier

let with_enable_xhp_class_modifier po b =
  { po with GlobalOptions.po_enable_xhp_class_modifier = b }

let disable_hh_ignore_error po = po.GlobalOptions.po_disable_hh_ignore_error

let interpret_soft_types_as_like_types po =
  po.GlobalOptions.po_interpret_soft_types_as_like_types

let with_interpret_soft_types_as_like_types po b =
  { po with GlobalOptions.po_interpret_soft_types_as_like_types = b }

let with_everything_sdt po b = { po with GlobalOptions.tco_everything_sdt = b }

let with_disallow_static_constants_in_default_func_args po b =
  {
    po with
    GlobalOptions.po_disallow_static_constants_in_default_func_args = b;
  }

let disallow_direct_superglobals_refs po =
  po.GlobalOptions.po_disallow_direct_superglobals_refs

let with_disallow_direct_superglobals_refs po b =
  { po with GlobalOptions.po_disallow_direct_superglobals_refs = b }

let with_stack_size po i = { po with GlobalOptions.po_stack_size = i }

let make
    ~auto_namespace_map
    ~codegen
    ~disable_lval_as_an_expression
    ~enable_class_level_where_clauses
    ~disable_legacy_soft_typehints
    ~disable_legacy_attribute_syntax
    ~const_default_func_args
    ~const_default_lambda_args
    ~disallow_silence
    ~const_static_props
    ~abstract_static_props
    ~disallow_func_ptrs_in_constants
    ~enable_xhp_class_modifier
    ~disable_xhp_element_mangling
    ~allow_unstable_features
    ~disable_xhp_children_declarations
    ~disable_hh_ignore_error
    ~interpret_soft_types_as_like_types
    ~is_systemlib
    ~disallow_direct_superglobals_refs
    ~stack_size =
  GlobalOptions.
    {
      default with
      po_auto_namespace_map = auto_namespace_map;
      po_codegen = codegen;
      po_disable_lval_as_an_expression = disable_lval_as_an_expression;
      po_enable_class_level_where_clauses = enable_class_level_where_clauses;
      po_disable_legacy_soft_typehints = disable_legacy_soft_typehints;
      po_disable_legacy_attribute_syntax = disable_legacy_attribute_syntax;
      po_const_default_func_args = const_default_func_args;
      po_const_default_lambda_args = const_default_lambda_args;
      po_disallow_silence = disallow_silence;
      tco_const_static_props = const_static_props;
      po_abstract_static_props = abstract_static_props;
      po_disallow_func_ptrs_in_constants = disallow_func_ptrs_in_constants;
      po_enable_xhp_class_modifier = enable_xhp_class_modifier;
      po_disable_xhp_element_mangling = disable_xhp_element_mangling;
      po_allow_unstable_features = allow_unstable_features;
      po_disable_xhp_children_declarations = disable_xhp_children_declarations;
      po_disable_hh_ignore_error = disable_hh_ignore_error;
      po_interpret_soft_types_as_like_types = interpret_soft_types_as_like_types;
      tco_is_systemlib = is_systemlib;
      po_disallow_direct_superglobals_refs = disallow_direct_superglobals_refs;
      po_stack_size = stack_size;
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

let to_rust_ffi_t po ~hhvm_compat_mode ~hhi_mode ~codegen =
  ( hhvm_compat_mode,
    hhi_mode,
    codegen,
    disable_lval_as_an_expression po,
    disable_legacy_soft_typehints po,
    const_static_props po,
    disable_legacy_attribute_syntax po,
    const_default_func_args po,
    abstract_static_props po,
    disallow_func_ptrs_in_constants po,
    enable_xhp_class_modifier po,
    disable_xhp_element_mangling po,
    disable_xhp_children_declarations po,
    const_default_lambda_args po,
    allow_unstable_features po,
    interpret_soft_types_as_like_types po,
    po.GlobalOptions.tco_is_systemlib,
    po.GlobalOptions.po_disallow_static_constants_in_default_func_args,
    disallow_direct_superglobals_refs po )

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = GlobalOptions.t [@@deriving show]

let auto_namespace_map = GlobalOptions.po_auto_namespace_map

let codegen = GlobalOptions.po_codegen

let deregister_php_stdlib = GlobalOptions.po_deregister_php_stdlib

let disallow_execution_operator = GlobalOptions.po_disallow_execution_operator

let disallow_toplevel_requires = GlobalOptions.po_disallow_toplevel_requires

let allow_goto = GlobalOptions.po_allow_goto

let default = GlobalOptions.default

let disable_nontoplevel_declarations =
  GlobalOptions.po_disable_nontoplevel_declarations

let disable_static_closures = GlobalOptions.po_disable_static_closures

let const_default_func_args = GlobalOptions.po_const_default_func_args

let with_const_default_func_args po b =
  { po with GlobalOptions.po_const_default_func_args = b }

let with_codegen po b = { po with GlobalOptions.po_codegen = b }

let with_disable_lval_as_an_expression po b =
  { po with GlobalOptions.po_disable_lval_as_an_expression = b }

let disable_lval_as_an_expression =
  GlobalOptions.po_disable_lval_as_an_expression

let rust_top_level_elaborator = GlobalOptions.po_rust_top_level_elaborator

let enable_class_level_where_clauses =
  GlobalOptions.po_enable_class_level_where_clauses

let disable_legacy_soft_typehints =
  GlobalOptions.po_disable_legacy_soft_typehints

let with_disable_legacy_soft_typehints po b =
  { po with GlobalOptions.po_disable_legacy_soft_typehints = b }

let disallowed_decl_fixmes = GlobalOptions.po_disallowed_decl_fixmes

let allow_new_attribute_syntax = GlobalOptions.po_allow_new_attribute_syntax

let with_allow_new_attribute_syntax po b =
  { po with GlobalOptions.po_allow_new_attribute_syntax = b }

let disable_legacy_attribute_syntax =
  GlobalOptions.po_disable_legacy_attribute_syntax

let with_disable_legacy_attribute_syntax po b =
  { po with GlobalOptions.po_disable_legacy_attribute_syntax = b }

let disallow_silence = GlobalOptions.po_disallow_silence

let const_static_props = GlobalOptions.tco_const_static_props

let with_const_static_props po b =
  { po with GlobalOptions.tco_const_static_props = b }

let abstract_static_props = GlobalOptions.po_abstract_static_props

let with_abstract_static_props po b =
  { po with GlobalOptions.po_abstract_static_props = b }

let disable_unset_class_const = GlobalOptions.po_disable_unset_class_const

let parser_errors_only = GlobalOptions.po_parser_errors_only

let with_parser_errors_only po b =
  { po with GlobalOptions.po_parser_errors_only = b }

let disallow_func_ptrs_in_constants =
  GlobalOptions.po_disallow_func_ptrs_in_constants

let with_disallow_func_ptrs_in_constants po b =
  { po with GlobalOptions.po_disallow_func_ptrs_in_constants = b }

let disable_xhp_element_mangling = GlobalOptions.po_disable_xhp_element_mangling

let with_disable_xhp_element_mangling po b =
  { po with GlobalOptions.po_disable_xhp_element_mangling = b }

let disable_xhp_children_declarations =
  GlobalOptions.po_disable_xhp_children_declarations

let with_disable_xhp_children_declarations po b =
  { po with GlobalOptions.po_disable_xhp_children_declarations = b }

let enable_xhp_class_modifier = GlobalOptions.po_enable_xhp_class_modifier

let with_enable_xhp_class_modifier po b =
  { po with GlobalOptions.po_enable_xhp_class_modifier = b }

let enable_first_class_function_pointers =
  GlobalOptions.po_enable_first_class_function_pointers

let with_enable_first_class_function_pointers po b =
  { po with GlobalOptions.po_enable_first_class_function_pointers = b }

let disable_modes = GlobalOptions.po_disable_modes

let disable_array = GlobalOptions.po_disable_array

let make
    ~auto_namespace_map
    ~codegen
    ~disallow_execution_operator
    ~disable_nontoplevel_declarations
    ~disable_static_closures
    ~disable_lval_as_an_expression
    ~enable_class_level_where_clauses
    ~disable_legacy_soft_typehints
    ~allow_new_attribute_syntax
    ~disable_legacy_attribute_syntax
    ~const_default_func_args
    ~disallow_silence
    ~const_static_props
    ~abstract_static_props
    ~disable_unset_class_const
    ~disallow_func_ptrs_in_constants
    ~enable_xhp_class_modifier
    ~disable_xhp_element_mangling
    ~disable_xhp_children_declarations
    ~rust_top_level_elaborator
    ~enable_first_class_function_pointers
    ~disable_modes
    ~disable_array =
  GlobalOptions.
    {
      default with
      po_auto_namespace_map = auto_namespace_map;
      po_codegen = codegen;
      po_disallow_execution_operator = disallow_execution_operator;
      po_disable_nontoplevel_declarations = disable_nontoplevel_declarations;
      po_disable_static_closures = disable_static_closures;
      po_disable_lval_as_an_expression = disable_lval_as_an_expression;
      po_enable_class_level_where_clauses = enable_class_level_where_clauses;
      po_disable_legacy_soft_typehints = disable_legacy_soft_typehints;
      po_allow_new_attribute_syntax = allow_new_attribute_syntax;
      po_disable_legacy_attribute_syntax = disable_legacy_attribute_syntax;
      po_const_default_func_args = const_default_func_args;
      po_disallow_silence = disallow_silence;
      tco_const_static_props = const_static_props;
      po_abstract_static_props = abstract_static_props;
      po_disable_unset_class_const = disable_unset_class_const;
      po_disallow_func_ptrs_in_constants = disallow_func_ptrs_in_constants;
      po_enable_xhp_class_modifier = enable_xhp_class_modifier;
      po_disable_xhp_element_mangling = disable_xhp_element_mangling;
      po_disable_xhp_children_declarations = disable_xhp_children_declarations;
      po_rust_top_level_elaborator = rust_top_level_elaborator;
      po_enable_first_class_function_pointers =
        enable_first_class_function_pointers;
      po_disable_modes = disable_modes;
      po_disable_array = disable_array;
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
    enable_first_class_function_pointers po,
    disable_modes po,
    disable_array po )

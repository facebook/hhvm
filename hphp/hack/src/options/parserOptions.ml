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

let rust_parser_errors = GlobalOptions.po_rust_parser_errors

let enable_constant_visibility_modifiers =
  GlobalOptions.po_enable_constant_visibility_modifiers

let enable_class_level_where_clauses =
  GlobalOptions.po_enable_class_level_where_clauses

let with_enable_constant_visibility_modifiers po b =
  { po with GlobalOptions.po_enable_constant_visibility_modifiers = b }

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

let disable_halt_compiler = GlobalOptions.po_disable_halt_compiler

let with_disable_halt_compiler po b =
  { po with GlobalOptions.po_disable_halt_compiler = b }

let make
    ~auto_namespace_map
    ~codegen
    ~disallow_execution_operator
    ~disable_nontoplevel_declarations
    ~disable_static_closures
    ~disable_lval_as_an_expression
    ~enable_constant_visibility_modifiers
    ~enable_class_level_where_clauses
    ~disable_legacy_soft_typehints
    ~allow_new_attribute_syntax
    ~disable_legacy_attribute_syntax
    ~const_default_func_args
    ~disallow_silence
    ~const_static_props
    ~abstract_static_props
    ~disable_unset_class_const
    ~disable_halt_compiler =
  GlobalOptions.
    {
      default with
      po_auto_namespace_map = auto_namespace_map;
      po_codegen = codegen;
      po_disallow_execution_operator = disallow_execution_operator;
      po_disable_nontoplevel_declarations = disable_nontoplevel_declarations;
      po_disable_static_closures = disable_static_closures;
      po_disable_lval_as_an_expression = disable_lval_as_an_expression;
      po_enable_constant_visibility_modifiers =
        enable_constant_visibility_modifiers;
      po_enable_class_level_where_clauses = enable_class_level_where_clauses;
      po_disable_legacy_soft_typehints = disable_legacy_soft_typehints;
      po_allow_new_attribute_syntax = allow_new_attribute_syntax;
      po_disable_legacy_attribute_syntax = disable_legacy_attribute_syntax;
      po_const_default_func_args = const_default_func_args;
      po_disallow_silence = disallow_silence;
      tco_const_static_props = const_static_props;
      po_abstract_static_props = abstract_static_props;
      po_disable_unset_class_const = disable_unset_class_const;
      po_disable_halt_compiler = disable_halt_compiler;
    }

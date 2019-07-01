(**
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
let allow_goto = GlobalOptions.po_allow_goto
let default = GlobalOptions.default
let disable_nontoplevel_declarations = GlobalOptions.po_disable_nontoplevel_declarations
let disable_static_closures = GlobalOptions.po_disable_static_closures
let disable_instanceof = GlobalOptions.po_disable_instanceof
let with_codegen po b =
  { po with GlobalOptions.po_codegen = b }
let with_disable_lval_as_an_expression po b =
  { po with GlobalOptions.po_disable_lval_as_an_expression = b }

let disable_lval_as_an_expression = GlobalOptions.po_disable_lval_as_an_expression
let setup_pocket_universes = GlobalOptions.setup_pocket_universes
let with_rust po b = { po with GlobalOptions.po_rust = b }
let rust = GlobalOptions.po_rust

let enable_constant_visibility_modifiers = GlobalOptions.po_enable_constant_visibility_modifiers

let with_enable_constant_visibility_modifiers po b =
  { po with GlobalOptions.po_enable_constant_visibility_modifiers = b }

let make
  ~auto_namespace_map
  ~codegen
  ~disallow_execution_operator
  ~disable_nontoplevel_declarations
  ~disable_static_closures
  ~disable_lval_as_an_expression
  ~disable_instanceof
  ~rust
  ~enable_constant_visibility_modifiers
= GlobalOptions.{
  default with
  po_auto_namespace_map = auto_namespace_map;
  po_codegen = codegen;
  po_disallow_execution_operator = disallow_execution_operator;
  po_disable_nontoplevel_declarations = disable_nontoplevel_declarations;
  po_disable_static_closures = disable_static_closures;
  po_disable_lval_as_an_expression = disable_lval_as_an_expression;
  po_disable_instanceof = disable_instanceof;
  po_rust = rust;
  po_enable_constant_visibility_modifiers = enable_constant_visibility_modifiers;
}

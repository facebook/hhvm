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
let deregister_php_stdlib = GlobalOptions.po_deregister_php_stdlib
let enable_hh_syntax_for_hhvm = GlobalOptions.po_enable_hh_syntax_for_hhvm
let disallow_execution_operator = GlobalOptions.po_disallow_execution_operator
let allow_goto = GlobalOptions.po_allow_goto
let enable_concurrent = GlobalOptions.po_enable_concurrent
let enable_await_as_an_expression = GlobalOptions.po_enable_await_as_an_expression
let default = GlobalOptions.default
let disable_nontoplevel_declarations = GlobalOptions.po_disable_nontoplevel_declarations
let disable_static_closures = GlobalOptions.po_disable_static_closures
let disable_instanceof = GlobalOptions.po_disable_instanceof
let with_hh_syntax_for_hhvm po b =
  { po with GlobalOptions.po_enable_hh_syntax_for_hhvm = b }
let with_enable_await_as_an_expression po b =
  { po with GlobalOptions.po_enable_await_as_an_expression = b }
let with_disable_lval_as_an_expression po b =
  { po with GlobalOptions.po_disable_lval_as_an_expression = b }

let enable_stronger_await_binding = GlobalOptions.po_enable_stronger_await_binding
let disable_lval_as_an_expression = GlobalOptions.po_disable_lval_as_an_expression
let setup_pocket_universes = GlobalOptions.setup_pocket_universes
let make
  ~auto_namespace_map
  ~enable_hh_syntax_for_hhvm
  ~enable_concurrent
  ~enable_await_as_an_expression
  ~disallow_execution_operator
  ~disable_nontoplevel_declarations
  ~disable_static_closures
  ~enable_stronger_await_binding
  ~disable_lval_as_an_expression
  ~disable_instanceof
= GlobalOptions.{
  default with
  po_auto_namespace_map = auto_namespace_map;
  po_enable_hh_syntax_for_hhvm = enable_hh_syntax_for_hhvm;
  po_enable_concurrent = enable_concurrent;
  po_enable_await_as_an_expression = enable_await_as_an_expression;
  po_disallow_execution_operator = disallow_execution_operator;
  po_disable_nontoplevel_declarations = disable_nontoplevel_declarations;
  po_disable_static_closures = disable_static_closures;
  po_enable_stronger_await_binding = enable_stronger_await_binding;
  po_disable_lval_as_an_expression = disable_lval_as_an_expression;
  po_disable_instanceof = disable_instanceof;
}

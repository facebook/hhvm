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
let disable_variable_variables = GlobalOptions.po_disable_variable_variables
let allow_goto = GlobalOptions.po_allow_goto
let enable_concurrent = GlobalOptions.po_enable_concurrent
let enable_await_as_an_expression = GlobalOptions.po_enable_await_as_an_expression
let default_mode = GlobalOptions.po_default_mode
let default = GlobalOptions.default
let disable_define = GlobalOptions.po_disable_define
let disable_nontoplevel_declarations = GlobalOptions.po_disable_nontoplevel_declarations
let with_hh_syntax_for_hhvm po b =
  { po with GlobalOptions.po_enable_hh_syntax_for_hhvm = b }
let with_enable_await_as_an_expression po b =
  { po with GlobalOptions.po_enable_await_as_an_expression = b }
let enable_stronger_await_binding = GlobalOptions.po_enable_stronger_await_binding

let make
  ~auto_namespace_map
  ~enable_hh_syntax_for_hhvm
  ~enable_concurrent
  ~enable_await_as_an_expression
  ~disallow_execution_operator
  ~disable_variable_variables
  ~disable_define
  ~disable_nontoplevel_declarations
  ~enable_stronger_await_binding = {
  default with
  GlobalOptions.po_auto_namespace_map = auto_namespace_map;
  GlobalOptions.po_enable_hh_syntax_for_hhvm = enable_hh_syntax_for_hhvm;
  GlobalOptions.po_enable_concurrent = enable_concurrent;
  GlobalOptions.po_enable_await_as_an_expression = enable_await_as_an_expression;
  GlobalOptions.po_disallow_execution_operator = disallow_execution_operator;
  GlobalOptions.po_disable_variable_variables = disable_variable_variables;
  GlobalOptions.po_disable_define = disable_define;
  GlobalOptions.po_disable_nontoplevel_declarations = disable_nontoplevel_declarations;
  GlobalOptions.po_enable_stronger_await_binding = enable_stronger_await_binding;
}

let is_ide_mode = ref false
let with_ide_mode ~f =
  is_ide_mode := true;
  Core_kernel.protect ~f ~finally:(fun () -> is_ide_mode := false)

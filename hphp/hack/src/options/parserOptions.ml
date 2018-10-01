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
let use_full_fidelity = GlobalOptions.po_use_full_fidelity
let enable_hh_syntax_for_hhvm = GlobalOptions.po_enable_hh_syntax_for_hhvm
let disallow_execution_operator = GlobalOptions.po_disallow_execution_operator
let disable_variable_variables = GlobalOptions.po_disable_variable_variables
let default = GlobalOptions.default
let with_hh_syntax_for_hhvm po b =
  { po with GlobalOptions.po_enable_hh_syntax_for_hhvm = b }
let with_disallow_execution_operator po b =
  { po with GlobalOptions.po_disallow_execution_operator = b }
let with_disable_variable_variables po b =
  { po with GlobalOptions.po_disable_variable_variables = b }

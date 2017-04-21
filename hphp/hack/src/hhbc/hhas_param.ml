(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type t = {
  param_name          : Litstr.id;
  param_is_reference  : bool;
  param_type_info     : Hhas_type_info.t option;
  param_default_value : (Label.t * Ast.expr) option
}

let make param_name param_is_reference param_type_info param_default_value =
  { param_name; param_is_reference; param_type_info; param_default_value }

let name p = p.param_name
let is_reference p = p.param_is_reference
let type_info p = p.param_type_info
let default_value p = p.param_default_value

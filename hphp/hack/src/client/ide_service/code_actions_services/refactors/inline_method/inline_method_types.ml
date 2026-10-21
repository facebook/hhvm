(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type param_info = {
  name: string;
  named: bool;
  default: Pos.t option;
  variadic: bool;
}

type method_info = {
  method_pos: Pos.t;
  block: Tast.block;
  params: param_info list;
  var_names: string list;
  has_void_return: bool;
  is_private: bool;
  return_cnt: int;
  last_stmt_is_return: bool;
  all_params_are_supported: bool;
}

type call_arg =
  | Positional of Pos.t
  | Named of string * Pos.t

type call_info = {
  callee_name: string;
  caller_name: string;
  call_stmt_pos: Pos.t;
  call_pos: Pos.t;
  call_args: call_arg list;
  arg_evaluation_order: int list;
}

type candidate = {
  call: call_info;
  callee: method_info;
  caller: method_info;
}

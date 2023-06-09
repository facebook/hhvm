(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type method_info = {
  method_pos: Pos.t;
  block: Tast.block;
  param_names: string list;
  var_names: string list;
  has_void_return: bool;
  is_private: bool;
  return_cnt: int;
  last_stmt_is_return: bool;
  all_params_are_normal: bool;
}

type call_info = {
  callee_name: string;
  caller_name: string;
  call_stmt_pos: Pos.t;
  call_pos: Pos.t;
  call_arg_positions: Pos.t list;
}

type candidate = {
  call: call_info;
  callee: method_info;
  caller: method_info;
}

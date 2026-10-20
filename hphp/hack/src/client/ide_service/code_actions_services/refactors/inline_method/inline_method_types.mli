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
      (** Only context-independent defaults can be copied. *)
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
      (** Inout, splat, and attributed parameters cannot be inlined. *)
}

type call_info = {
  callee_name: string;
  caller_name: string;
  call_stmt_pos: Pos.t;  (** Helpful for formatting *)
  call_pos: Pos.t;
  call_args: (string option * Pos.t) list;
      (** Argument names and expression positions in source order. *)
  arg_evaluation_order: int list;
      (** HackC preserves side effects in source order but can defer pure argument reads. *)
}

type candidate = {
  call: call_info;
      (** Information about the location of the call
   * to the method to be inlined *)
  callee: method_info;  (** The method to inline *)
  caller: method_info;  (** the method we are inlining the callee into *)
}

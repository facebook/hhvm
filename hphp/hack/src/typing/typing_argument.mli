(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type dyn_func_kind =
  | Supportdyn_function
  | Like_function

val check_argument_type_against_parameter_type :
  ?is_single_argument:bool ->
  dynamic_func:dyn_func_kind option ->
  ignore_readonly:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Pos.t ->
  Typing_defs.locl_ty ->
  Typing_env_types.env
  * (Typing_defs.locl_ty * Typing_defs.locl_ty) option
  * bool

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(**
 * Logic shared by the tast check and the logger
 *)

type class_use_kind =
  | Static_method_call
  | New

type class_use_info =
  class_use_kind * Pos.t * Typing_warning.Safe_abstract.t list

(**
* If `expr` is the typ of thing for which we can
* ask if it is abstract-safe (`Static_method_call or `New)
* return `Some class_use_info` *)
val calc_warnings :
  Tast_env.env ->
  Tast.expr ->
  current_method:Tast.method_ option ->
  class_use_info option

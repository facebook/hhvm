(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val type_fun :
  Provider_context.t ->
  Relative_path.t ->
  string ->
  (Tast.def * Typing_inference_env.t_global_with_pos) option

val type_class :
  Provider_context.t ->
  Relative_path.t ->
  string ->
  (Tast.def * Typing_inference_env.t_global_with_pos list) option

val type_record_def :
  Provider_context.t -> Relative_path.t -> string -> Tast.def option

val check_typedef :
  Provider_context.t -> Relative_path.t -> string -> Tast.def option

val check_const :
  Provider_context.t -> Relative_path.t -> string -> Tast.def option

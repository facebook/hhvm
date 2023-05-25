(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type class_member_standalone_check_env

(** Create an environment from a class that can be used to subsequently
  type-check members of that class individually *)
val make_class_member_standalone_check_env :
  Provider_context.t ->
  Nast.class_ ->
  (Typing_env_types.env * class_member_standalone_check_env) option

(** Type-checks the given method of the class that was specific when creating
  the [class_member_standalone_check_env] *)
val method_def_standalone :
  class_member_standalone_check_env ->
  string ->
  (Tast.method_ list * Typing_inference_env.t_global_with_pos) option

(** This is a helper for [Typing_toplevel.class_def]. Call that instead. *)
val class_def : Provider_context.t -> Nast.class_ -> Tast.class_ option

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Search through the input list, and any unions directly-recursively contained in the input list,
    and return those that satisfy f, and those that do not, separately.
 *)
val partition_union :
  f:(Typing_defs.locl_ty -> bool) ->
  Typing_env_types.env ->
  Typing_defs.locl_ty list ->
  Typing_defs.locl_ty list * Typing_defs.locl_ty list

(** If input is dynamic | t or t | dynamic then return Some t.
    Otherwise return None.
    If do_not_solve_likes=false then solve lower bounds such as ~int <: #0
    in order to expose the dynamic for stripping.
 *)
val try_strip_dynamic :
  ?accept_intersections:bool ->
  ?do_not_solve_likes:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty option

val try_strip_dynamic_from_union :
  ?accept_intersections:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty list ->
  (Typing_defs.locl_ty * Typing_defs.locl_ty list) option

(** If input is dynamic | t or t | dynamic then return t,
    otherwise return type unchanged. *)
val strip_dynamic :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

(** Detect types that look like (t1 & dynamic) | t2 and convert to
   ~t2 & (t1 | t2). Also in function returns.
*)
val recompose_like_type :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

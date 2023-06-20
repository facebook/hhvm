(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_env_types

(*****************************************************************************)
(* Functions dealing with old style local environment *)
(*****************************************************************************)

val get_all_locals : env -> Typing_per_cont_env.t

val get_cont_option :
  env -> Typing_continuations.t -> Typing_per_cont_env.per_cont_entry option

val drop_cont : env -> Typing_continuations.t -> env

val drop_conts : env -> Typing_continuations.t list -> env

val replace_cont :
  env ->
  Typing_continuations.t ->
  Typing_per_cont_env.per_cont_entry option ->
  env

val restore_conts_from :
  env -> Typing_per_cont_env.t -> Typing_continuations.t list -> env

val restore_and_merge_conts_from :
  env ->
  join_pos:Pos.t ->
  Typing_per_cont_env.t ->
  Typing_continuations.t list ->
  env

val update_next_from_conts :
  env -> join_pos:Pos.t -> Typing_continuations.t list -> env

val save_and_merge_next_in_cont :
  env -> join_pos:Pos.t -> Typing_continuations.t -> env

val move_and_merge_next_in_cont :
  env -> join_pos:Pos.t -> Typing_continuations.t -> env

val union :
  env ->
  join_pos:Pos.t ->
  Typing_local_types.local ->
  Typing_local_types.local ->
  env * Typing_local_types.local

val union_by_cont : env -> join_pos:Pos.t -> local_env -> local_env -> env

val union_contextopts :
  join_pos:Pos.t ->
  env ->
  Typing_per_cont_env.per_cont_entry option ->
  Typing_per_cont_env.per_cont_entry option ->
  env * Typing_per_cont_env.per_cont_entry option

val union_lenvs :
  env -> join_pos:Pos.t -> local_env -> local_env -> local_env -> env

val union_lenv_list :
  env -> join_pos:Pos.t -> local_env -> local_env list -> env

(* When entering control flow structures, some
 * preexisting continuations must be stashed away and then restored
 * on exiting those control flow structures.
 *
 * For example, on entering a loop, preexisting 'break' and 'continue'
 * continuations from any enclosing loops must be stashed away so as not to
 * interfere with them. *)
val stash_and_do :
  env -> Typing_continuations.t list -> (env -> env * 'a) -> env * 'a

val env_with_empty_fakes : env -> env

val has_next : env -> bool

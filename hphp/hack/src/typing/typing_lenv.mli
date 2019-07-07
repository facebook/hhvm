(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Functions dealing with old style local environment *)
(*****************************************************************************)

val get_all_locals :
  Typing_env.env -> Typing_per_cont_env.t
val get_cont_option :
  Typing_env.env ->
  Typing_continuations.t -> Typing_per_cont_env.per_cont_entry option
val drop_cont :
  Typing_env.env ->
  Typing_continuations.t -> Typing_env.env
val drop_conts :
  Typing_env.env ->
  Typing_continuations.t list -> Typing_env.env
val replace_cont :
  Typing_env.env ->
  Typing_continuations.t ->
  Typing_per_cont_env.per_cont_entry option -> Typing_env.env
val restore_conts_from :
  Typing_env.env ->
  Typing_per_cont_env.t ->
  Typing_continuations.t list -> Typing_env.env
val restore_and_merge_conts_from :
  Typing_env.env ->
  Typing_per_cont_env.t ->
  Typing_continuations.t list -> Typing_env.env
val update_next_from_conts :
  Typing_env.env ->
  Typing_continuations.t list -> Typing_env.env
val save_and_merge_next_in_cont :
  Typing_env.env ->
  Typing_continuations.t -> Typing_env.env
val move_and_merge_next_in_cont :
  Typing_env.env ->
  Typing_continuations.t -> Typing_env.env
val union:
  Typing_env.env ->
  Typing_local_types.local ->
  Typing_local_types.local ->
  Typing_env.env * Typing_local_types.local
val union_by_cont :
  Typing_env.env ->
  Typing_env.local_env ->
  Typing_env.local_env -> Typing_env.env
val union_contextopts :
  Typing_env.env ->
  Typing_per_cont_env.per_cont_entry option ->
  Typing_per_cont_env.per_cont_entry option ->
  Typing_env.env * Typing_per_cont_env.per_cont_entry option
val union_lenvs :
  Typing_env.env ->
  Typing_env.local_env ->
  Typing_env.local_env ->
  Typing_env.local_env -> Typing_env.env
val union_lenv_list :
  Typing_env.env ->
  Typing_env.local_env ->
  Typing_env.local_env list -> Typing_env.env
val union_envs :
  Typing_env.env ->
  Typing_env.env ->
  Typing_env.env ->
  Typing_env.env
(* When entering control flow structures, some
 * preexisting continuations must be stashed away and then restored
 * on exiting those control flow structures.
 *
 * For example, on entering a loop, preexisting 'break' and 'continue'
 * continuations from any enclosing loops must be stashed away so as not to
 * interfere with them. *)
val stash_and_do :
  Typing_env.env ->
  Typing_continuations.t list ->
  (Typing_env.env ->
  Typing_env.env * 'a) -> Typing_env.env * 'a
val env_with_empty_fakes :
  Typing_env.env -> Typing_env.env
val has_next :
  Typing_env.env -> bool

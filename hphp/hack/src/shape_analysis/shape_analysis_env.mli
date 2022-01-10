(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types
module LMap = Local_id.Map

(** Generates a fresh variable entity *)
val fresh_var : unit -> entity_

(** Initialise shape analysis environment *)
val init : Tast.saved_env -> constraint_ list -> entity LMap.t -> env

(** Record a shape analysis constraint *)
val add_constraint : env -> constraint_ -> env

(** Ignore all existing constraints. The intention of this is to prevent
    unnecessary duplication of constraints when multiple environments need to
    be merged. *)
val reset_constraints : env -> env

(** Find an entity that a local variable points to *)
val get_local : env -> Local_id.t -> entity

(** Set an entity to a local variable *)
val set_local : env -> Local_id.t -> entity -> env

(** The first environment is the parent environment. The others are combined.
    This is useful in branching code. *)
val union : env -> env -> env -> env

val stash_and_do : env -> Typing_continuations.t list -> (env -> env) -> env

val update_next_from_conts : env -> Typing_continuations.t list -> env

val drop_cont : env -> Typing_continuations.t -> env

val restore_conts_from : env -> from:lenv -> Typing_continuations.t list -> env

val move_and_merge_next_in_cont : env -> Typing_continuations.t -> env

val save_and_merge_next_in_cont : env -> Typing_continuations.t -> env

val loop_continuation :
  Typing_continuations.t ->
  env_before_iteration:env ->
  env_after_iteration:env ->
  env

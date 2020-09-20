(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Functions dealing with continuation based flow typing of local variables *)
(*****************************************************************************)

module C = Typing_continuations
module CMap = C.Map

type per_cont_entry = {
  (* Local types per continuation. For example, the local types of the
   * break continuation correspond to the local types that there were at the
   * last encountered break in the current scope. These are kept to be merged
   * at the appropriate merge points. *)
  local_types: Typing_local_types.t;
  (* Fake members are used when we want member variables to be treated like
   * locals. We want to handle the following:
   * if($this->x) {
   *   ... $this->x ...
   * }
   * The trick consists in replacing $this->x with a "fake" local. So that
   * all the logic that normally applies to locals is applied in cases like
   * this. Hence the name: FakeMembers.
   * All the fake members are thrown away at the first call.
   * We keep the invalidated fake members for better error messages.
   *)
  fake_members: Typing_fake_members.t;
  (* Type parameter environment
   * Lower and upper bounds on generic type parameters and abstract types
   * For constraints of the form Tu <: Tv where both Tu and Tv are type
   * parameters, we store an upper bound for Tu and a lower bound for Tv.
   * Contrasting with tenv and subst, bounds are *assumptions* for type
   * inference, not conclusions.
   *)
  tpenv: Type_parameter_env.t;
}

type t = per_cont_entry Typing_continuations.Map.t

val initial_locals : per_cont_entry -> t

val empty_entry : per_cont_entry

(* Get a continuation wrapped in Some, or None if not found *)
val get_cont_option : C.t -> t -> per_cont_entry option

(** Get all continuations present in an environment *)
val all_continuations : t -> C.t list

(* Add the key, value pair to the continuation named 'name'
 * If the continuation doesn't exist, create it *)
val add_to_cont : C.t -> Local_id.t -> Typing_local_types.local -> t -> t

val update_cont_entry : C.t -> t -> (per_cont_entry -> per_cont_entry) -> t

(* remove a local from a continuation if it exists. Otherwise do nothing. *)
val remove_from_cont : C.t -> Local_id.t -> t -> t

(* Drop a continuation. If the continuation is absent, the map remains
 * unchanged. *)
val drop_cont : C.t -> t -> t

val drop_conts : C.t list -> t -> t

val replace_cont : C.t -> per_cont_entry option -> t -> t

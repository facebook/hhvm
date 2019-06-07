(**
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
  local_types        : Typing_local_types.t;

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
  fake_members       : Typing_fake_members.t;
}

type t = per_cont_entry Typing_continuations.Map.t

exception Continuation_not_found of string

val empty_locals : t
val initial_locals : t

val empty_entry : per_cont_entry

(* Get a continuation wrapped in Some, or None if not found *)
val get_cont_option :
  C.t -> t -> per_cont_entry option

(* Get a continuation, or raise Continuation_not_found if not found *)
val get_cont_exn : C.t -> t -> per_cont_entry

(* Add the key, value pair to the continuation named 'name'
 * If the continuation doesn't exist, create it *)
val add_to_cont :
  C.t -> Local_id.t -> Typing_local_types.local -> t -> t

val update_cont_entry :
  C.t -> t -> (per_cont_entry -> per_cont_entry) -> t

(* remove a local from a continuation if it exists. Otherwise do nothing. *)
val remove_from_cont :
  C.t -> Local_id.t -> t -> t

(* Drop a continuation. If the continuation is absent, the map remains
 * unchanged. *)
val drop_cont : C.t -> t -> t
val drop_conts : C.t list -> t -> t
val replace_cont :
  C.t -> per_cont_entry option -> t -> t

(* Takes two continuation maps m1 (`locals`) and m2 (`source_locals`) and
 * a continuation c (`cont`) and basically perform m1[c] <= m2[c].
 *
 * When entering certain control flow structures, certain
 * preexisting continuations must be stashed away and then _restored_
 * via this function on exiting those control flow structures.
 *
 * For example, on entering a loop, preexisting 'break' and 'continue'
 * continuations from any enclosing loops must be stashed away so as not to
 * interfere with them. *)
val restore_conts_from :
  t ->
  from:t ->
  C.t list ->
  t

type 'a locals_merge_fn =
  'a -> Typing_local_types.local -> Typing_local_types.local -> 'a * Typing_local_types.local

(* Same as restore_conts_from, except continuations from the 'from' locals
 * are unioned with the continuations from the current environment. This is
 * used after typechecking a try-catch-finally statement, as continuations
 * may be used outside of this statement *)
val restore_and_merge_conts_from :
  'a ->
  'a locals_merge_fn ->
  t ->
  from:t ->
  C.t list -> 'a * t

(* Merge all continuations in the provided list and update the next
 * continuation with the result.
 *
 * This is used at certain merge points in the control flow. For example,
 * after a loop, we want to merge the 'break' continuation into the 'next'
 * continuation, or at the beginning of a loop block, we merge the 'continue'
 * continuation into the 'next' continuation. *)
val update_next_from_conts :
  'a ->
  'a locals_merge_fn ->
  t ->
  CMap.key list -> 'a * t

(* After this call, the provided continuation will be the union of
 * itself and the next continuation.
 *
 * This is used at split points in the control flow.
 * For example, if we encounter a function call that may throw an exception,
 * we "save" the 'next' continuation by "merging" it into the 'catch'
 * continuation.
 *)
val save_and_merge_next_in_cont :
  'a ->
  'a locals_merge_fn ->
  t ->
  CMap.key -> 'a * t

(* union the provided continuation with the next continuation and store
 * the result in the provided continuation. Finally, remove the next
 * continuation.
 *
 * This is used at jump points in the control flow.
 * For example, when we encounter a 'continue' statement, we "move" the 'next'
 * continuation by "merging" it into the 'continue' continuation.
 *)
val move_and_merge_next_in_cont :
  'a ->
  'a locals_merge_fn ->
  t ->
  CMap.key -> 'a * t

(* Unions two context options. We call "context" here a map from locals to
 * types.
 * Intersect the set of keys, and for each key in both contexts, union their
 * associated types.
 * More formally, the union c1 & c2 of contexts
 * c1 and c2 is defined as:
 * (x: T) belongs to c1 & c2 iif there exist T1, T2 such that:
 *   - (x: T1) belongs to c1
 *   - (x: T2) belongs to c2
 *   - T = T1 | T2
 *
 * Example:
 *   context1 = { x: A, y: B }
 *   context2 = { z: C, y: D }
 *   union context1 context2 = { y: (B|D) }
 *)
val union_opts :
  'a locals_merge_fn ->
  'a ->
  per_cont_entry option ->
  per_cont_entry option -> 'a * per_cont_entry option

(* union all continuations pairwise from two continuation maps.
 * This is used at certain merge points in the control flow,
 * typically after an `if` or a `switch` statement.
 *
 * Example:
 *   locals1 = {
 *     Next:     { x: A, y: B },
 *     Continue: { x: A, y: B },
 *     Break:    { x: A, y: B },
 *   }
 *   locals2 = {
 *     Next:     { z: C, y: D },
 *     Continue: { z: C, y: D },
 *     Catch:    { z: C, y: D },
 *   }
 *   union_by_cont locals1 locals2 = {
 *     Next:     { y: (B|D) },
 *     Continue: { y: (B|D) },
 *     Break:    { x: A, y: B },
 *     Catch:    { z: C, y: D },
 *   }
 *)
val union_by_cont :
  'a ->
  'a locals_merge_fn ->
  t ->
  t -> 'a * t

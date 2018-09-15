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

include Typing_env_types_sig.S
module C = Typing_continuations
module CMap = C.Map
module LMap = Local_id.Map

exception Continuation_not_found of string

val empty_locals : local_types
val initial_locals : local_types

(* Get a continuation wrapped in Some, or None if not found *)
val get_cont_option :
  C.t -> local_types -> local_id_map option

(* Get a continuation, or raise Continuation_not_found if not found *)
val get_cont : C.t -> local_types -> local_id_map

(* Return the first existing continuation from the list of provided
 * continuations. If none exists, raise Continuation_not_found.
 * This is used when checking todos after typechecking the body of a function,
 * when the locals may be in the Next (no return statement), Exit (the function
 * has return statements) or Catch (the function always throws) continuation. *)
val try_get_conts : C.t list -> local_types -> local_id_map

(* Add the key, value pair to the continuation named 'name'
 * If the continuation doesn't exist, create it *)
val add_to_cont :
  C.t -> Local_id.t -> local -> local_types -> local_types

(* remove a local from a continuation if it exists. Otherwise do nothing. *)
val remove_from_cont :
  C.t -> Local_id.t -> local_types -> local_types

(* Drop a continuation. If the continuation is absent, the map remains
 * unchanged. *)
val drop_cont : C.t -> local_types -> local_types
val drop_conts : C.t list -> local_types -> local_types
val replace_cont :
  C.t -> local_id_map option -> local_types -> local_types

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
  local_types ->
  from:local_types ->
  C.t list ->
  local_types

(* Same as restore_conts_from, except continuations from the 'from' locals
 * are unioned with the continuations from the current environment. This is
 * used after typechecking a try-catch-finally statement, as continuations
 * may be used outside of this statement *)
val restore_and_merge_conts_from :
  'a ->
  ('a -> local -> local -> 'a * local) ->
  local_types ->
  from:local_types ->
  C.t list -> 'a * local_types

(* Merge all continuations in the provided list and update the next
 * continuation with the result.
 *
 * This is used at certain merge points in the control flow. For example,
 * after a loop, we want to merge the 'break' continuation into the 'next'
 * continuation, or at the beginning of a loop block, we merge the 'continue'
 * continuation into the 'next' continuation. *)
val update_next_from_conts :
  'a ->
  ('a -> local -> local -> 'a * local) ->
  local_types ->
  CMap.key list -> 'a * local_types

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
  ('a -> local -> local -> 'a * local) ->
  local_types ->
  CMap.key -> 'a * local_types

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
  ('a -> local -> local -> 'a * local) ->
  local_types ->
  CMap.key -> 'a * local_types

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
  ('a -> local -> local -> 'a * local) ->
  'a ->
  local_id_map option ->
  local_id_map option -> 'a * local_id_map option

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
  ('a -> local -> local -> 'a * local) ->
  local_types ->
  local_types -> 'a * local_types

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
open Typing_env_types
module C = Typing_continuations
module LEnvC = Typing_per_cont_env

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
val restore_conts_from : LEnvC.t -> from:LEnvC.t -> C.t list -> LEnvC.t

type 'a locals_merge_fn =
  'a ->
  Typing_local_types.local ->
  Typing_local_types.local ->
  'a * Typing_local_types.local

(* Same as restore_conts_from, except continuations from the 'from' locals
 * are unioned with the continuations from the current environment. This is
 * used after typechecking a try-catch-finally statement, as continuations
 * may be used outside of this statement *)
val restore_and_merge_conts_from :
  env ->
  env locals_merge_fn ->
  LEnvC.t ->
  from:LEnvC.t ->
  C.t list ->
  env * LEnvC.t

(* Merge all continuations in the provided list and update the next
 * continuation with the result.
 *
 * This is used at certain merge points in the control flow. For example,
 * after a loop, we want to merge the 'break' continuation into the 'next'
 * continuation, or at the beginning of a loop block, we merge the 'continue'
 * continuation into the 'next' continuation. *)
val update_next_from_conts :
  env -> env locals_merge_fn -> LEnvC.t -> C.Map.key list -> env * LEnvC.t

(* After this call, the provided continuation will be the union of
 * itself and the next continuation.
 *
 * This is used at split points in the control flow.
 * For example, if we encounter a function call that may throw an exception,
 * we "save" the 'next' continuation by "merging" it into the 'catch'
 * continuation.
 *)
val save_and_merge_next_in_cont :
  env -> env locals_merge_fn -> LEnvC.t -> C.Map.key -> env * LEnvC.t

(* union the provided continuation with the next continuation and store
 * the result in the provided continuation. Finally, remove the next
 * continuation.
 *
 * This is used at jump points in the control flow.
 * For example, when we encounter a 'continue' statement, we "move" the 'next'
 * continuation by "merging" it into the 'continue' continuation.
 *)
val move_and_merge_next_in_cont :
  env -> env locals_merge_fn -> LEnvC.t -> C.Map.key -> env * LEnvC.t

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
  env locals_merge_fn ->
  env ->
  LEnvC.per_cont_entry option ->
  LEnvC.per_cont_entry option ->
  env * LEnvC.per_cont_entry option

val is_sub_opt_entry :
  (env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool) ->
  env ->
  LEnvC.per_cont_entry option ->
  LEnvC.per_cont_entry option ->
  bool

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
  env -> env locals_merge_fn -> LEnvC.t -> LEnvC.t -> env * LEnvC.t

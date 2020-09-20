(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

(* Tracks the different types of mutability of a given local variable.
  See typing_mutability.ml for a description of the fields.
*)
module LMap = Local_id.Map

type mut_type =
  | Mutable
  | Borrowed
  | MaybeMutable
  | Immutable
[@@deriving eq]

type mutability = Pos.t * mut_type

let to_string_ = function
  | Mutable -> "mutably-owned"
  | Borrowed -> "mutably-borrowed"
  | MaybeMutable -> "maybe-mutable"
  | Immutable -> "immutable"

let to_string (_, mut) = to_string_ mut

(* Mapping from local variables to their mutability
  Local mutability is stored in the local environment.
  Mutability flavor of the local is determined on the first assignment.
  - Borrowed/maybemutable locals cannot be reassigned because it will create
    an alias for the value
  - immutable locals can be reassigned because they
    don't have single-reference restriction
  - owned mutable locals can be reassigned because rhs is always something that
    transfers the ownership of the value (Rx\mutable or Rx\move) so
    single-reference restriction will still be preserved
  - for locals - arguments to move\freeze calls we still keep value in map
    as MutableUnset to keep mutability flavor information
*)
type mutability_env = mutability Local_id.Map.t

(* Given two mutability maps, intersect them. *)
let intersect_mutability
    (parent_mut : mutability_env) (m1 : mutability_env) (m2 : mutability_env) :
    mutability_env =
  let merge _id v1_opt v2_opt =
    match (v1_opt, v2_opt) with
    | (Some (p1, mut1), Some (p2, mut2)) ->
      let assumed_mut =
        if equal_mut_type mut1 mut2 then
          mut1
        else (
          Errors.inconsistent_mutability
            p1
            (to_string_ mut1)
            (Some (p2, to_string_ mut2));
          mut1
        )
      in
      Some (p1, assumed_mut)
    | (Some v, None)
    | (None, Some v) ->
      Some v
    | _ -> None
  in
  (* ensure not conflicting mutability flavors in child scopes *)
  let acc = LMap.merge merge m1 m2 in
  (* ensure no conflicting flavors in child and parent scopes *)
  let acc = LMap.merge merge parent_mut acc in
  acc

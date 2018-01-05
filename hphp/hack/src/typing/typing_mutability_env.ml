(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
(* Tracks the different types of mutability of a given local variable.
  See typing_mutability.mli for a description of the fields.
*)
module LMap = Local_id.Map
type mut_type = Mutable | Borrowed | Const
type mutability = Pos.t * mut_type

let to_string = function
| _, Mutable -> "mutably-owned"
| _, Borrowed -> "mutably-borrowed"
| _, Const -> "constant"

(* Mapping from local variables to their mutability
  Local mutability is stored in the local environment.
  Once a variable is frozen, we remove it from the map,
  and it is treated like any immutable variable in its
  enclosing function.
*)
type mutability_env = mutability LMap.t

(* Given two mutability maps, intersect them. *)
let intersect_mutability
  (parent_mut : mutability_env)
  (m1 : mutability_env)
  (m2 : mutability_env)
  : mutability_env =
  (* Check for any variables that were frozen in one scope but not the other *)
  LMap.iter
  begin fun id _ ->
    match LMap.get id m1, LMap.get id m2 with
    | Some (p, _), None
    | None, Some (p, _) ->
      Errors.frozen_in_incorrect_scope p;
    | _ -> ()
  end parent_mut;
  LMap.fold
  begin fun id (p1, mut1) acc ->
    match LMap.get id m2 with
    (* This means it's mutable in one scope but not the other.
    We raise errors previously if it was frozen in one scope but not the other.
    1. If the variable was defined in one scope but not the other, and also
    not defined in the parent, then we can safely ignore it.
    2. If the varibale was defined in the parent scope, and somehow disappeared
    from scope in one scope but not the other, we raised an error already.
    In all cases, we can just ignore it here. *)
    | None -> acc
    | Some (_, mut2) ->
      match mut1, mut2 with
      | _ when mut1 = mut2 ->
        LMap.add id (p1, mut1) acc
      (* It's not possible for two variables to both be mutable AND have different
        mutabilities in the same scope: Owned, Borrowed, and Const persist
        for the rest of the function body.
      *)
      | _ -> assert false
  end parent_mut m1

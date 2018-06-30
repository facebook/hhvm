(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* Tracks the different types of mutability of a given local variable.
  See typing_mutability.ml for a description of the fields.
*)
module LMap = Local_id.Map
type mut_type = Mutable | Borrowed | MaybeMutable
type mutability = Pos.t * mut_type

let to_string = function
| _, Mutable -> "mutably-owned"
| _, Borrowed -> "mutably-borrowed"
| _, MaybeMutable -> "maybe-mutable"

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
  let merge ~keep_left _id v1_opt v2_opt =
    match v1_opt, v2_opt with
    | Some (p1, mut1), Some (_, mut2) ->
      let assumed_mut =
        (* do a conservative merge for mutability values *)
        begin match mut1, mut2 with
        | Mutable, Borrowed | Borrowed, Mutable -> Borrowed
        | _ -> if mut1 = mut2 then mut1 else MaybeMutable
        end in
      Some (p1, assumed_mut)
    | Some l, None when keep_left -> Some l
    | _ -> None in
  (* intersect variables in child maps, keep only entries that exists in both *)
  let acc = LMap.merge (merge ~keep_left:false) m1 m2 in
  (* combine result with parent env preserving items from parent *)
  let acc = LMap.merge (merge ~keep_left:true) parent_mut acc in
  acc

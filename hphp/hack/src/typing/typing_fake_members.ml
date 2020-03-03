(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Validation blame: call or lambda *)
type blame =
  | Blame_call of Pos.t
  | Blame_lambda of Pos.t

(* Fake member validation.
 *   Valid valid
 * means that identifiers in [valid] are valid fake members
 *   Invalidated { valid, invalid, blame }
 * means that identifiers in [invalid] have been invalidated
 * by the call or lambda described by [blame], but [valid]
 * are more recent valid fake members. For example
 *
 *   // Valid []
 *   if ($x::f !== null) {
 *     // Valid ["$x::f"]
 *     foo(); // P1
 *     // Invalidated { valid = []; invalid = ["$x::f"]; blame = Blame_call P1 }
 *     if ($y::g !== null) {
 *       // Invalidated { valid = ["$y::g"]; invalid = ["$x::f"]; blame = Blame_call P1 }
 *       $f = () ==>  // P2
 *       // Invalidated { valid = []; invalid = ["$x::f"; "$y::g"]; blame = Blame_lambda P2 }
         ...
 *)
type t =
  | Valid of Local_id.Set.t
  | Invalidated of {
      valid: Local_id.Set.t;
      (* Non-empty and disjoint from valid *)
      invalid: Local_id.Set.t;
      (* cause of invalidation: call or lambda *)
      blame: blame;
    }

let empty = Valid Local_id.Set.empty

(* Combine validation information at a join point *)
let join fake1 fake2 =
  match (fake1, fake2) with
  | (Valid ids1, Valid ids2) -> Valid (Local_id.Set.inter ids1 ids2)
  | (Invalidated { valid = v2; invalid; blame }, Valid v1)
  | (Valid v1, Invalidated { valid = v2; invalid; blame }) ->
    let valid = Local_id.Set.inter v1 v2 in
    let invalid = Local_id.Set.union invalid (Local_id.Set.diff v2 valid) in
    Invalidated { valid; invalid; blame }
  | ( Invalidated { valid = v1; invalid = i1; blame },
      Invalidated { valid = v2; invalid = i2; _ } ) ->
    Invalidated
      {
        valid = Local_id.Set.inter v1 v2;
        invalid = Local_id.Set.union i1 i2;
        blame;
      }

(* Does fake1 entail fake2? *)
let sub fake1 fake2 =
  match (fake1, fake2) with
  | (Valid ids1, Valid ids2) -> Local_id.Set.subset ids2 ids1
  | (Invalidated { valid = v2; invalid; _ }, Valid v1) ->
    Local_id.Set.subset v1 v2
    && Local_id.Set.is_empty (Local_id.Set.diff invalid v2)
  | (Valid v1, Invalidated { valid = v2; _ }) -> Local_id.Set.subset v2 v1
  | ( Invalidated { valid = v1; invalid = i1; _ },
      Invalidated { valid = v2; invalid = i2; _ } ) ->
    Local_id.Set.subset v2 v1 && Local_id.Set.subset i1 i2

let is_valid fake lid =
  match fake with
  | Invalidated { valid; _ }
  | Valid valid ->
    Local_id.Set.mem lid valid

let is_invalid fake lid =
  match fake with
  | Invalidated { invalid; blame; _ } ->
    if Local_id.Set.mem lid invalid then
      Some blame
    else
      None
  | Valid _ -> None

let forget fake blame =
  match fake with
  | Valid valid when Local_id.Set.is_empty valid -> fake
  | Valid valid ->
    Invalidated { valid = Local_id.Set.empty; invalid = valid; blame }
  | Invalidated { valid; invalid; _ } ->
    Invalidated
      {
        valid = Local_id.Set.empty;
        invalid = Local_id.Set.union valid invalid;
        blame;
      }

let add fake lid =
  match fake with
  | Valid valid -> Valid (Local_id.Set.add lid valid)
  | Invalidated ({ valid; _ } as info) ->
    Invalidated { info with valid = Local_id.Set.add lid valid }

let blame_as_log_value blame =
  match blame with
  | Blame_call p -> Typing_log_value.(make_map [("Blame_call", pos_as_value p)])
  | Blame_lambda p ->
    Typing_log_value.(make_map [("Blame_lambda", pos_as_value p)])

let as_log_value fake =
  match fake with
  | Valid valid ->
    Typing_log_value.(make_map [("Valid", local_id_set_as_value valid)])
  | Invalidated { valid; invalid; blame } ->
    Typing_log_value.(
      make_map
        [
          ( "Invalidated",
            make_map
              [
                ("valid", local_id_set_as_value valid);
                ("invalid", local_id_set_as_value invalid);
                ("blame", blame_as_log_value blame);
              ] );
        ])

let make_id obj_name member_name =
  let obj_name =
    match obj_name with
    | (_, Aast.This) -> Typing_defs.this
    | (_, Aast.Lvar (_, x)) -> x
    | _ -> assert false
  in
  Local_id.make_unscoped (Local_id.to_string obj_name ^ "->" ^ member_name)

let make_static_id cid member_name =
  let class_name = Nast.class_id_to_str cid in
  Local_id.make_unscoped (class_name ^ "::" ^ member_name)

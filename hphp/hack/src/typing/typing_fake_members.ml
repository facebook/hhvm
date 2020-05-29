(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Reason = Typing_reason

module S = struct
  type t = Local_id.t * Reason.blame

  let compare (lid1, _) (lid2, _) = compare lid1 lid2
end

(* A set that treats blame as metadata *)
module BlameSet = struct
  include Caml.Set.Make (S)

  let find_by_lid_opt lid blame_set =
    filter (fun (lid', _) -> Local_id.equal lid lid') blame_set |> choose_opt

  let member_by_lid lid blame_set =
    exists (fun (lid', _) -> Local_id.equal lid lid') blame_set

  let attach_blame blame blame_set =
    map (fun (lid, _) -> (lid, blame)) blame_set
end

(* Fake member validation.
 *   Valid valid
 * means that identifiers in [valid] are valid fake members
 *   Invalidated { valid, invalid }
 * means that identifiers in [invalid] have been invalidated
 * by a call, lambda, or assignment described by the blame attached to
 * the element, but [valid] are more recent valid fake members. For example
 *
 *   // Valid []
 *   if ($x::f !== null) { // P0
 *     // Valid [("$x::f", Blame_out_of_scope P0)]
 *     foo(); // P1
 *     // Invalidated { valid = []; invalid = [("$x::f", Blame_call P1)] }
 *     if ($y::g !== null) { // P2
 *       // Invalidated { valid = [("$y::g", Blame_out_of_scope P2)]; invalid = [("$x::f", Blame_call P1)] }
 *       $f = () ==>  // P3
 *       // Invalidated { valid = []; invalid = [("$x::f", Blame_lambda P3); ("$y::g", Blame_lambda P2)] }
         ...
 *)
type t =
  | Valid of BlameSet.t
  | Invalidated of {
      valid: BlameSet.t;
      (* Non-empty and disjoint from valid. *)
      invalid: BlameSet.t;
    }

let empty = Valid BlameSet.empty

(* Combine validation information at a join point *)
let join fake1 fake2 =
  match (fake1, fake2) with
  | (Valid ids1, Valid ids2) -> Valid (BlameSet.inter ids1 ids2)
  | (Invalidated { valid = v2; invalid }, Valid v1)
  | (Valid v1, Invalidated { valid = v2; invalid }) ->
    let valid = BlameSet.inter v1 v2 in
    let invalid = BlameSet.union invalid (BlameSet.diff v2 valid) in
    Invalidated { valid; invalid }
  | ( Invalidated { valid = v1; invalid = i1 },
      Invalidated { valid = v2; invalid = i2 } ) ->
    Invalidated { valid = BlameSet.inter v1 v2; invalid = BlameSet.union i1 i2 }

(* Does fake1 entail fake2? *)
let sub fake1 fake2 =
  match (fake1, fake2) with
  | (Valid ids1, Valid ids2) -> BlameSet.subset ids2 ids1
  | (Invalidated { valid = v2; invalid; _ }, Valid v1) ->
    BlameSet.subset v1 v2 && BlameSet.is_empty (BlameSet.diff invalid v2)
  | (Valid v1, Invalidated { valid = v2; _ }) -> BlameSet.subset v2 v1
  | ( Invalidated { valid = v1; invalid = i1; _ },
      Invalidated { valid = v2; invalid = i2; _ } ) ->
    BlameSet.subset v2 v1 && BlameSet.subset i1 i2

let is_valid fake lid =
  match fake with
  | Invalidated { valid; _ }
  | Valid valid ->
    BlameSet.member_by_lid lid valid

let is_invalid fake lid =
  match fake with
  | Invalidated { invalid; _ } ->
    Option.map (BlameSet.find_by_lid_opt lid invalid) snd
  | Valid _ -> None

let forget fake blame =
  match fake with
  | Valid valid when BlameSet.is_empty valid -> fake
  | Valid valid ->
    Invalidated
      { valid = BlameSet.empty; invalid = BlameSet.attach_blame blame valid }
  | Invalidated { valid; invalid; _ } ->
    Invalidated
      {
        valid = BlameSet.empty;
        invalid = BlameSet.attach_blame blame (BlameSet.union valid invalid);
      }

let forget_prefixed (fake_members : t) prefix_lid blame : t =
  let is_prefixed (fake_id, _blame) =
    String.is_prefix
      ~prefix:(Local_id.to_string prefix_lid ^ "->")
      (Local_id.to_string fake_id)
  in
  let invalidate_prefixed valid =
    let (prefixed, others) = BlameSet.partition is_prefixed valid in
    (BlameSet.attach_blame blame prefixed, others)
  in
  match fake_members with
  | Valid valid when BlameSet.is_empty valid -> fake_members
  | Valid valid ->
    let (invalid, valid) = invalidate_prefixed valid in
    Invalidated { valid; invalid }
  | Invalidated { valid; invalid } ->
    let (invalidated, valid) = invalidate_prefixed valid in
    Invalidated { valid; invalid = BlameSet.union invalidated invalid }

let add fake lid pos =
  match fake with
  | Valid valid ->
    Valid (BlameSet.add (lid, Reason.(Blame (pos, BSout_of_scope))) valid)
  | Invalidated ({ valid; _ } as info) ->
    Invalidated
      {
        info with
        valid = BlameSet.add (lid, Reason.(Blame (pos, BSout_of_scope))) valid;
      }

let blame_as_log_value (Reason.Blame (p, blame_source)) =
  match blame_source with
  | Reason.BScall ->
    Typing_log_value.(make_map [("Blame_call", pos_as_value p)])
  | Reason.BSlambda ->
    Typing_log_value.(make_map [("Blame_lambda", pos_as_value p)])
  | Reason.BSassignment ->
    Typing_log_value.(make_map [("Blame_assigment", pos_as_value p)])
  | Reason.BSout_of_scope ->
    Typing_log_value.(make_map [("Blame_out_of_scope", pos_as_value p)])

let as_log_value fake =
  let log_blame_set_as_value set =
    Typing_log_value.make_map
      (List.map (BlameSet.elements set) (fun (lid, blame_opt) ->
           ( Typing_log_value.local_id_as_string lid,
             blame_as_log_value blame_opt )))
  in
  match fake with
  | Valid valid ->
    Typing_log_value.(make_map [("Valid", log_blame_set_as_value valid)])
  | Invalidated { valid; invalid } ->
    Typing_log_value.(
      make_map
        [
          ( "Invalidated",
            make_map
              [
                ("valid", log_blame_set_as_value valid);
                ("invalid", log_blame_set_as_value invalid);
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

(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Nast
open Ast_defs
open List
module Reason = Typing_reason

let internal_error s =
  failwith ("Something (internal) went wrong while typing a regex string: " ^ s)

let rec int_keys p top bottom acc_i =
  if top <= bottom then acc_i
  else int_keys p (top - 1) bottom ((SFlit_int (p, string_of_int top)) :: acc_i)

let rec keys' p count names name_numbers acc =
  match name_numbers with
  | [] -> (int_keys p count 0 []) @ acc
  | head :: _ ->
    keys' p (head - 1) (tl names) (tl name_numbers)
      ((SFlit_str (p, hd names)) :: acc)

(*
 *  Shape keys for our match type. For re"Hel(\D)(?'o'\D)", this is
 * [ SFlit_int (p, "0"); SFlit_int (p, "1"); SFlit_int (p, 'o') ].
 *
 *)
let keys p s =
  (* Compile `re`-prefixed string for use in Pcre functions *)
  let pattern = Pcre.regexp s in
  (* For re"Hel(\D)(?'o'\D)", this is 2. *)
  let count =
    try Pcre.capturecount pattern
    with Pcre.Error (Pcre.InternalError s) -> internal_error s in
  (* For re"Hel(\D)(?'o'\D)", this is ['o']. *)
  let names =
    try List.rev (Array.to_list (Pcre.names pattern)) (* Shape field order! *)
    with Pcre.Error (Pcre.InternalError s) -> internal_error s in
  (*  For re"Hel(\D)(?'o'\D)", this is [2] *)
  let name_numbers =
    try List.map (Pcre.get_stringnumber pattern) names
    with Invalid_argument s -> internal_error s in

  (* Any Regex\Match will contain the entire matched substring at key 0. *)
  SFlit_int (p, "0") :: (keys' p count names name_numbers [])

let type_match p s =
  let sft =
    { sft_optional = false; sft_ty = Reason.Rregex p, Tprim Tstring; } in
  let keys = keys p s in
  let shape_map = List.fold_left (fun acc name -> ShapeMap.add name sft acc)
    ShapeMap.empty keys in
  Reason.Rregex p, Tshape (FieldsFullyKnown, shape_map)

let type_pattern (p, e_) =
  match e_ with
  | String s ->
    let match_type = type_match p s in
    Reason.Rregex p,
      Tabstract (AKnewtype (Naming_special_names.Regex.tPattern,
      [match_type]),
      Some (Reason.Rregex p, Tprim Tstring))
  | _ -> failwith "Should have caught non-Ast.String prefixed expression!"

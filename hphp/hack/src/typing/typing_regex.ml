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
module Reason = Typing_reason

exception Empty_regex_pattern
exception Missing_delimiter
exception Invalid_global_option

let internal_error s =
  failwith ("Something (internal) went wrong while typing a regex string: " ^ s)

(*  bottom is non-inclusive *)
let rec int_keys p top bottom acc_i =
  if top <= bottom then acc_i
  else int_keys p (top - 1) bottom ((SFlit_int (p, string_of_int top)) :: acc_i)

(* Assumes that names_numbers is sorted in DECREASING order of numbers *)
let rec keys_aux p top names_numbers acc =
  match names_numbers with
  | [] -> (int_keys p top 0 []) @ acc
  | (name, number) :: t ->
    keys_aux p (number - 1) t
      (SFlit_str (p, name) :: ((int_keys p top number []) @ acc))

(*
 *  Any shape keys for our match type except 0. For re"Hel(\D)(?'o'\D)", this is
 * [ SFlit_int (p, "1"); SFlit_int (p, 'o') ].
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
    try Array.to_list (Pcre.names pattern)
    with Pcre.Error (Pcre.InternalError s) -> internal_error s in
  (*  For re"Hel(\D)(?'o'\D)", this is [2] *)
  let numbers =
    try List.map (Pcre.get_stringnumber pattern) names
    with Invalid_argument s -> internal_error s in
  let names_numbers = List.combine names numbers in
  let names_numbers_sorted =
    List.sort (fun nn1 nn2 -> ~- (Pervasives.compare (snd nn1) (snd nn2)))
      names_numbers in
  keys_aux p count names_numbers_sorted []

let type_match p s =
  let sft_0 =
    { sft_optional = false; sft_ty = Reason.Rregex p, Tprim Tstring; } in
  let sft =
    { sft_optional = true; sft_ty = Reason.Rregex p, Tprim Tstring; } in
  let keys = keys p s in
  let shape_map = List.fold_left (fun acc key -> ShapeMap.add key sft acc)
    ShapeMap.empty keys in
  (* Any Regex\Match will contain the entire matched substring at key 0.
    For now, as the native impl omits non-matching captures,
    all fields but the 0 field will be optional. *)
  let shape_map = ShapeMap.add (SFlit_int (p, "0")) sft_0 shape_map in
  Reason.Rregex p, Tshape (FieldsFullyKnown, shape_map)

let check_global_options s =
  String.iter (fun c ->
    match c with
    | 'i' | 'm' | 's' | 'x' | 'A' | 'D' | 'S' | 'U' | 'X' | 'u' -> ()
    | _ -> raise Invalid_global_option) s

let rec find_delimiter s desired from_i =
  match String.index_from_opt s from_i desired with
  | Some i ->
    if i <> 0 && s.[i - 1] = '\\'
    then find_delimiter s desired (i + 1)
    else Some i
  | None -> None

let check_and_strip_delimiters s =
  (*  Non-alphanumeric, non-whitespace, non-backslash characters are delimiter-eligible *)
  let delimiter = Str.regexp "[^a-zA-Z0-9\t\n\r\x0b\x0c \\]" in
  let length = String.length s in
  if length = 0 then raise Empty_regex_pattern else
  let first = s.[0] in
  if Str.string_match delimiter (String.make 1 first) 0
  then begin
    let desired =
      match first with
      | '(' -> ')'
      | '[' -> ']'
      | '{' -> '}'
      | '<' -> '>'
      | _ -> first
    in
    match find_delimiter (String.sub s 1 (length - 1)) desired 0 with
    | Some i -> (* i is 0-indexed from the second character in s *)
      check_global_options (String.sub s (i + 2) ((length - 2) - i));
      String.sub s 1 i
    | None -> raise Missing_delimiter
  end else raise Missing_delimiter

let type_pattern (p, e_) =
  match e_ with
  | String s ->
    let s = check_and_strip_delimiters s in
    let match_type = type_match p s in
    Reason.Rregex p,
      Tabstract (AKnewtype (Naming_special_names.Regex.tPattern,
      [match_type]),
      Some (Reason.Rregex p, Tprim Tstring))
  | _ -> failwith "Should have caught non-Ast.String prefixed expression!"

(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core

module A = Ast
module H = Hhbc_ast

(* TODO: This list needs to be exhaustive *)
let get_kind p = Int64.of_int @@
  match p with
  | "void" -> 0
  | "int" -> 1
  | "bool" -> 2
  | "float" -> 3
  | "string" -> 4
  | "resource" -> 5
  | "num" -> 6
  | "noreturn" -> 8
  | "arraykey" -> 7
  | "mixed" -> 9
  | "shape" -> 14
  | _ -> 99999999 (* umm? *)

let shape_field_name = function
  | A.SFlit ((_, s))
  | A.SFclass_const (_, (_, s)) -> s

let rec shape_field_to_instr_lit_list sf =
  let name = shape_field_name sf.A.sf_name in
  (* TODO: Optional? *)
  let hint = sf.A.sf_hint in
  let value = H.Array (1, [H.String "value"; hint_to_type_constant hint]) in
  [H.String name; value]

and shape_info_to_instr_lit si =
  let l = si.A.si_shape_field_list in
  H.Array (List.length l,
      List.concat @@ List.map ~f:shape_field_to_instr_lit_list l)

and hint_to_type_constant_list h =
  match snd h with
  | A.Happly ((_, s), []) ->
    [H.String "kind"; H.Int (get_kind s)]
  | A.Hshape (si) ->
    [H.String "kind"; H.Int (get_kind "shape");
     H.String "fields"; shape_info_to_instr_lit si]
  | _ -> [H.String "kind"; H.NYI "type_constants"]

and hint_to_type_constant h =
  let l = hint_to_type_constant_list h in
  let count = List.length l in
  let count =
    if count mod 2 = 0
    then count / 2
    else failwith "hint_to_type_constant - odd length"
  in
  H.Array (count, l)

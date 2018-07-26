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

(* TODO[T31009719]: Analyze pattern_expr using PCRE and add shape fields accordingly *)
let names (p, _pattern_expr_) = [ SFlit_int (p, "0") ]

let type_match e =
  let p = fst e in
  let sft =
    { sft_optional = false; sft_ty = Reason.Rregex p, Tprim Tstring; } in
  let names = names e in
  let shape_map = List.fold_left (fun acc name -> ShapeMap.add name sft acc)
    ShapeMap.empty names in
  Reason.Rregex p, Tshape (FieldsFullyKnown, shape_map)

let type_pattern e =
  let p = fst e in
  (Reason.Rregex p,
    Tabstract (AKnewtype (Naming_special_names.Regex.tPattern, [type_match e]),
      Some (Reason.Rregex p, Tprim Tstring)))

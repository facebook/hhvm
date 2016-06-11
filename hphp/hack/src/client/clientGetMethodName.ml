(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_json

let get_result_type res =
  match res.SymbolOccurrence.type_ with
  | SymbolOccurrence.Class -> "class"
  | SymbolOccurrence.Method _ -> "method"
  | SymbolOccurrence.Function -> "function"
  | SymbolOccurrence.LocalVar -> "local"
  | SymbolOccurrence.Property _ -> "property"
  | SymbolOccurrence.ClassConst _ -> "class_const"
  | SymbolOccurrence.Typeconst _ -> "typeconst"
  | SymbolOccurrence.GConst -> "global_const"

let to_json = function
  | Some (res, _) ->
    JSON_Object [
      "name",           JSON_String res.SymbolOccurrence.name;
      "result_type",    JSON_String (get_result_type res);
      "pos",            Pos.json (res.SymbolOccurrence.pos);
      "internal_error", JSON_Bool false;
    ]
  | None -> JSON_Object [ "internal_error", JSON_Bool false ]

let print_json res =
  print_endline (Hh_json.json_to_string (to_json res))

let print_readable = function
  | Some (res, _) ->
    let line, start, end_ = Pos.info_pos res.SymbolOccurrence.pos in
    Printf.printf "Name: %s, type: %s, position: line %d, characters %d-%d\n"
      res.SymbolOccurrence.name
      (get_result_type res)
      line start end_
  | None -> ()

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res

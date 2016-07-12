(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Hh_json

let to_json x =
  JSON_Array (List.map x begin function (symbol, definition) ->
    let definition_pos, definition_span, definition_id =
      Option.value_map definition
      ~f:(fun x -> Pos.json x.SymbolDefinition.pos,
                   Pos.multiline_json x.SymbolDefinition.span,
                   x.SymbolDefinition.id)
      ~default:(JSON_Null, JSON_Null, None)
    in
    let definition_id = Option.value_map definition_id
      ~f:(fun x -> JSON_String x) ~default:JSON_Null
    in
    JSON_Object [
      "name",           JSON_String symbol.SymbolOccurrence.name;
      "result_type",    JSON_String
        (ClientGetMethodName.get_result_type symbol);
      "pos",            Pos.json (symbol.SymbolOccurrence.pos);
      "definition_pos", definition_pos;
      "definition_span", definition_span;
      "definition_id", definition_id;
    ]
  end)

let print_json res =
  print_endline (Hh_json.json_to_string (to_json res))

let print_readable x =
  List.iter x begin function (symbol, definition) ->
    Printf.printf "Name: %s, type: %s, position: %s"
      symbol.SymbolOccurrence.name
      (ClientGetMethodName.get_result_type symbol)
      (Pos.string_no_file symbol.SymbolOccurrence.pos);
    Option.iter definition begin fun x ->
      Printf.printf ", defined: %s" (Pos.string_no_file x.SymbolDefinition.pos);
      Printf.printf ", definition span: %s"
        (Pos.multiline_string_no_file x.SymbolDefinition.pos)
    end;
    print_newline ()
  end

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res

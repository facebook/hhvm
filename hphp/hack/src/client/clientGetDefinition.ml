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
        (SymbolOccurrence.(kind_to_string symbol.type_));
      "pos",            Pos.json (symbol.SymbolOccurrence.pos);
      "definition_pos", definition_pos;
      "definition_span", definition_span;
      "definition_id", definition_id;
    ]
  end)

let print_json res =
  print_endline (Hh_json.json_to_string (to_json res))

let print_readable ?short_pos:(short_pos=false) x =
  List.iter x begin function (occurrence, definition) ->
    let open SymbolOccurrence in
    let {name; type_; pos;} = occurrence in
    Printf.printf "name: %s, kind: %s, span: %s, definition: "
      name (kind_to_string type_) (Pos.string_no_file pos);
    begin match definition with
    | None ->  Printf.printf "None\n"
    | Some definition ->
      print_newline ();
      FileOutline.print_def ~short_pos " " definition
    end;
    print_newline ()
  end

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res

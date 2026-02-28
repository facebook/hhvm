(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Hh_json

let opt_field ~v_opt ~label ~f =
  Option.value_map v_opt ~f:(fun x -> [(label, f x)]) ~default:[]

(* There are fields that Nuclide doesn't use anymore, but the RPC framework
 * still requires them in responses. Stub them with some default values in the
 * meantime *)
let deprecated_pos_field = Pos.json (Pos.to_absolute Pos.none)

let infer_type_response_to_json (type_string, type_json) =
  Hh_json.JSON_Object
    ([("type", string_opt type_string); ("pos", deprecated_pos_field)]
    @
    match type_json with
    | Some json -> [("full_type", json_of_string json)]
    | _ -> [])

let infer_type_error_response_to_json
    ( actual_type_string,
      actual_type_json,
      expected_type_string,
      expected_type_json ) =
  Hh_json.JSON_Object
    (List.filter_map
       ~f:Fn.id
       [
         Some ("actual_type", string_opt actual_type_string);
         Option.map
           ~f:(fun ty -> ("full_actual_type", json_of_string ty))
           actual_type_json;
         Some ("expected_type", string_opt expected_type_string);
         Option.map
           ~f:(fun ty -> ("full_expected_type", json_of_string ty))
           expected_type_json;
       ])

let tast_holes_response_to_json ~print_file holes =
  let printer pos =
    if print_file then
      Pos.to_absolute pos |> Pos.multiline_json
    else
      Pos.multiline_json_no_filename pos
  in
  let f
      ( actual_type_str,
        actual_type_json,
        expected_type_str,
        expected_type_json,
        pos ) =
    Hh_json.JSON_Object
      [
        ("actual_type", Hh_json.string_ actual_type_str);
        ("full_actual_type", json_of_string actual_type_json);
        ("expected_type", Hh_json.string_ expected_type_str);
        ("full_expected_type", json_of_string expected_type_json);
        ("pos", printer pos);
      ]
  in
  Hh_json.JSON_Array (List.map ~f holes)

let identify_symbol_response_to_json results =
  let get_definition_data = function
    | Some x ->
      let { SymbolDefinition.pos; span; _ } = x in
      let pos = Pos.json pos in
      let span = Pos.multiline_json span in
      let id = SymbolDefinition.identifier x |> string_opt in
      (pos, span, id)
    | None -> (JSON_Null, JSON_Null, JSON_Null)
  in
  let symbol_to_json (occurrence, definition) =
    let (definition_pos, definition_span, definition_id) =
      get_definition_data definition
    in
    SymbolOccurrence.(
      JSON_Object
        [
          ("name", JSON_String occurrence.name);
          ("result_type", JSON_String (kind_to_string occurrence.type_));
          ("pos", Pos.json occurrence.pos);
          ("definition_pos", definition_pos);
          ("definition_span", definition_span);
          ("definition_id", definition_id);
        ])
  in
  JSON_Array (List.map results ~f:symbol_to_json)

let rec definition_to_json def =
  SymbolDefinition.(
    let modifiers =
      JSON_Array
        (List.map def.modifiers ~f:(fun x -> JSON_String (string_of_modifier x)))
    in
    let children =
      opt_field
        ~v_opt:
          (match def.kind with
          | Classish { members; _ } -> Some members
          | _ -> None)
        ~label:"children"
        ~f:outline_response_to_json
    in
    let params =
      opt_field ~v_opt:def.params ~label:"params" ~f:outline_response_to_json
    in
    let docblock =
      opt_field ~v_opt:def.docblock ~label:"docblock" ~f:(fun x ->
          JSON_String x)
    in
    JSON_Object
      ([
         ("kind", JSON_String (string_of_kind def.kind));
         ("name", JSON_String def.name);
         ("id", SymbolDefinition.identifier def |> string_opt);
         ("position", Pos.json def.pos);
         ("span", Pos.multiline_json def.span);
         ("modifiers", modifiers);
       ]
      @ children
      @ params
      @ docblock))

and outline_response_to_json x =
  Hh_json.JSON_Array (List.map x ~f:definition_to_json)

let highlight_references_response_to_json l =
  JSON_Array
    (List.map l ~f:(fun { Ide_api_types.st; ed } ->
         let (line, char_start) =
           File_content.Position.line_column_one_based st
         in
         let (_, char_end) = File_content.Position.line_column_one_based ed in
         Hh_json.JSON_Object
           [
             ("line", Hh_json.int_ line);
             ("char_start", Hh_json.int_ char_start);
             ("char_end", Hh_json.int_ char_end);
           ]))

let print_json json = Hh_json.json_to_string json |> print_endline

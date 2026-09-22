(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let opt_field ~v_opt ~label ~f =
  Option.value_map v_opt ~f:(fun x -> [(label, f x)]) ~default:[]

let string_opt = function
  | Some s -> `String s
  | None -> `Null

(* There are fields that Nuclide doesn't use anymore, but the RPC framework
 * still requires them in responses. Stub them with some default values in the
 * meantime *)
let deprecated_pos_field = Pos.json (Pos.to_absolute Pos.none)

let infer_type_response_to_json (type_string, type_json) =
  `Assoc
    ([("type", string_opt type_string); ("pos", deprecated_pos_field)]
    @
    match type_json with
    | Some json -> [("full_type", Yojson.Safe.from_string json)]
    | _ -> [])

let infer_type_error_response_to_json
    ( actual_type_string,
      actual_type_json,
      expected_type_string,
      expected_type_json ) =
  `Assoc
    (List.filter_map
       ~f:Fn.id
       [
         Some ("actual_type", string_opt actual_type_string);
         Option.map
           ~f:(fun ty -> ("full_actual_type", Yojson.Safe.from_string ty))
           actual_type_json;
         Some ("expected_type", string_opt expected_type_string);
         Option.map
           ~f:(fun ty -> ("full_expected_type", Yojson.Safe.from_string ty))
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
    `Assoc
      [
        ("actual_type", `String actual_type_str);
        ("full_actual_type", Yojson.Safe.from_string actual_type_json);
        ("expected_type", `String expected_type_str);
        ("full_expected_type", Yojson.Safe.from_string expected_type_json);
        ("pos", printer pos);
      ]
  in
  `List (List.map ~f holes)

let identify_symbol_response_to_json results =
  let get_definition_data = function
    | Some x ->
      let { Symbol_definition.pos; span; _ } = x in
      let pos = Pos.json pos in
      let span = Pos.multiline_json span in
      let id = Symbol_definition.identifier x |> string_opt in
      (pos, span, id)
    | None -> (`Null, `Null, `Null)
  in
  let symbol_to_json (occurrence, definition) =
    let (definition_pos, definition_span, definition_id) =
      get_definition_data definition
    in
    Symbol_occurrence.(
      `Assoc
        [
          ("name", `String occurrence.name);
          ("result_type", `String (kind_to_string occurrence.type_));
          ("pos", Pos.json occurrence.pos);
          ("definition_pos", definition_pos);
          ("definition_span", definition_span);
          ("definition_id", definition_id);
        ])
  in
  `List (List.map results ~f:symbol_to_json)

let rec definition_to_json def =
  Symbol_definition.(
    let modifiers =
      `List
        (List.map def.modifiers ~f:(fun x -> `String (string_of_modifier x)))
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
      opt_field ~v_opt:def.docblock ~label:"docblock" ~f:(fun x -> `String x)
    in
    `Assoc
      ([
         ("kind", `String (string_of_kind def.kind));
         ("name", `String def.name);
         ("id", Symbol_definition.identifier def |> string_opt);
         ("position", Pos.json def.pos);
         ("span", Pos.multiline_json def.span);
         ("modifiers", modifiers);
       ]
      @ children
      @ params
      @ docblock))

and outline_response_to_json x = `List (List.map x ~f:definition_to_json)

let highlight_references_response_to_json l =
  `List
    (List.map l ~f:(fun { Ide_api_types.st; ed } ->
         let (line, char_start) =
           File_content.Position.line_column_one_based st
         in
         let (_, char_end) = File_content.Position.line_column_one_based ed in
         `Assoc
           [
             ("line", `Int line);
             ("char_start", `Int char_start);
             ("char_end", `Int char_end);
           ]))

let print_json json = Yojson.Safe.to_string json |> print_endline

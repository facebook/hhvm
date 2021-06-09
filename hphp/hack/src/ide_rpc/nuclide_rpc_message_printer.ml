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

let deprecated_int_field = Hh_json.int_ 0

let deprecated_bool_field = JSON_Bool false

(* Instead of "assert false" *)
let should_not_happen = JSON_Object [("this_should", JSON_String "not_happen")]

let infer_type_response_to_json (type_string, type_json) =
  Hh_json.JSON_Object
    ( [("type", opt_string_to_json type_string); ("pos", deprecated_pos_field)]
    @
    match type_json with
    | Some json -> [("full_type", json_of_string json)]
    | _ -> [] )

let infer_type_error_response_to_json
    ( actual_type_string,
      actual_type_json,
      expected_type_string,
      expected_type_json ) =
  Hh_json.JSON_Object
    (List.filter_map
       ~f:Fn.id
       [
         Some ("actual_type", opt_string_to_json actual_type_string);
         Option.map
           ~f:(fun ty -> ("full_actual_type", json_of_string ty))
           actual_type_json;
         Some ("expected_type", opt_string_to_json expected_type_string);
         Option.map
           ~f:(fun ty -> ("full_expected_type", json_of_string ty))
           expected_type_json;
       ])

let tast_holes_response_to_json holes =
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
        ("pos", Pos.multiline_json_no_filename pos);
      ]
  in
  Hh_json.JSON_Array (List.map ~f holes)

let identify_symbol_response_to_json results =
  let get_definition_data = function
    | Some x ->
      SymbolDefinition.(
        let pos = Pos.json x.pos in
        let span = Pos.multiline_json x.span in
        let id = opt_string_to_json x.id in
        (pos, span, id))
    | None -> (JSON_Null, JSON_Null, JSON_Null)
  in
  let result_type x =
    SymbolOccurrence.(
      match x.type_ with
      | Class _ -> "class"
      | Method _ -> "method"
      | Record -> "record"
      | Function -> "function"
      | LocalVar -> "local"
      | Property _ -> "property"
      | XhpLiteralAttr _ -> "xhp_literal_attribute"
      | ClassConst _ -> "class_const"
      | Typeconst _ -> "typeconst"
      | GConst -> "global_const"
      | Attribute _ -> "attribute"
      | EnumClassLabel _ -> "enum_class_label")
  in
  let symbol_to_json (occurrence, definition) =
    let (definition_pos, definition_span, definition_id) =
      get_definition_data definition
    in
    SymbolOccurrence.(
      JSON_Object
        [
          ("name", JSON_String occurrence.name);
          ("result_type", JSON_String (result_type occurrence));
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
        ~v_opt:def.children
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
      ( [
          ("kind", JSON_String (string_of_kind def.kind));
          ("name", JSON_String def.name);
          ("id", opt_string_to_json def.id);
          ("position", Pos.json def.pos);
          ("span", Pos.multiline_json def.span);
          ("modifiers", modifiers);
        ]
      @ children
      @ params
      @ docblock ))

and outline_response_to_json x =
  Hh_json.JSON_Array (List.map x ~f:definition_to_json)

let coverage_levels_response_to_json spans =
  let opt_coverage_level_to_string =
    Option.value_map ~f:Coverage_level.string_of_level ~default:"default"
  in
  let span_to_json (color, text) =
    JSON_Object
      [
        ("color", JSON_String (opt_coverage_level_to_string color));
        ("text", JSON_String text);
      ]
  in
  JSON_Array (List.map spans ~f:span_to_json)

let symbol_by_id_response_to_json = function
  | Some def -> definition_to_json def
  | None -> JSON_Null

let find_references_response_to_json = function
  | None -> JSON_Array []
  | Some (symbol_name, references) ->
    let entries =
      List.map references ~f:(fun x ->
          Ide_api_types.(
            Hh_json.JSON_Object
              [
                ("name", Hh_json.JSON_String symbol_name);
                ("filename", Hh_json.JSON_String x.range_filename);
                ("line", Hh_json.int_ x.file_range.st.line);
                ("char_start", Hh_json.int_ x.file_range.st.column);
                ("char_end", Hh_json.int_ (x.file_range.ed.column - 1));
              ]))
    in
    Hh_json.JSON_Array entries

let highlight_references_response_to_json l =
  JSON_Array
    (List.map l ~f:(fun x ->
         Ide_api_types.(
           Hh_json.JSON_Object
             [
               ("line", Hh_json.int_ x.st.line);
               ("char_start", Hh_json.int_ x.st.column);
               ("char_end", Hh_json.int_ (x.ed.column - 1));
             ])))

let print_json json = Hh_json.json_to_string json |> print_endline

(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Hh_core
open Ide_message
open Ide_parser_utils
open Hh_json

(* There are fields that Nuclide doesn't use anymore, but the RPC framework
 * still requires them in responses. Stub them with some default values in the
 * meantime *)
let deprecated_pos_field = Pos.json (Pos.to_absolute Pos.none)
let deprecated_int_field = Hh_json.int_ 0
let deprecated_bool_field = JSON_Bool false

(* Instead of "assert false" *)
let should_not_happen = JSON_Object [
  ("this_should", JSON_String "not_happen");
]

let autocomplete_response_to_json x =
  let param_to_json x = JSON_Object [
    ("name", JSON_String x.callable_param_name);
    ("type", JSON_String x.callable_param_type);
    ("variadic", deprecated_bool_field);
  ] in

  let callable_details_to_json x = JSON_Object [
    ("return_type", JSON_String x.return_type);
    ("params", JSON_Array (List.map x.callable_params ~f:param_to_json));
    ("min_arity", deprecated_int_field);
  ] in

  let callable_details_to_json = function
    | None -> []
    | Some x -> [("func_details", callable_details_to_json x)] in

  let autocomplete_response_to_json x = JSON_Object ([
    ("name", JSON_String x.autocomplete_item_text);
    ("type", JSON_String x.autocomplete_item_type);
    ("pos", deprecated_pos_field);
    ("expected_ty", deprecated_bool_field)
  ] @ (callable_details_to_json x.callable_details)) in

  JSON_Array (List.map x ~f:autocomplete_response_to_json)

let infer_type_response_to_json x =
  Hh_json.JSON_Object (
    [
      ("type", opt_string_to_json x.type_string);
      ("pos", deprecated_pos_field)
    ] @
    (match x.type_json with
    | Some json -> [("full_type", json_of_string json)]
    | _ -> [])
  )

let identify_symbol_response_to_json results =

  let get_definition_data = function
    | Some x ->
        let open SymbolDefinition in
        let pos = Pos.json x.pos in
        let span = Pos.multiline_json x.span in
        let id = opt_string_to_json x.id in
        pos, span, id
    | None -> (JSON_Null, JSON_Null, JSON_Null)
  in

  let result_type x =
    let open SymbolOccurrence in
    match x.type_ with
    | Class -> "class"
    | Method _ -> "method"
    | Function -> "function"
    | LocalVar -> "local"
    | Property _ -> "property"
    | ClassConst _ -> "class_const"
    | Typeconst _ -> "typeconst"
    | GConst -> "global_const"
  in

  let symbol_to_json { occurrence; definition; } =
    let definition_pos, definition_span, definition_id =
      get_definition_data definition in
    let open SymbolOccurrence in
    JSON_Object [
      "name", JSON_String occurrence.name;
      "result_type", JSON_String (result_type occurrence);
      "pos", Pos.json occurrence.pos;
      "definition_pos", definition_pos;
      "definition_span", definition_span;
      "definition_id", definition_id;
    ]
  in
  JSON_Array (List.map results ~f:symbol_to_json)

let rec definition_to_json def =
  let open SymbolDefinition in

  let modifiers = JSON_Array (List.map def.modifiers
    ~f:(fun x -> JSON_String (string_of_modifier x))) in
  let children =
    opt_field def.children "children" outline_response_to_json in
  let params =
    opt_field def.params "params" outline_response_to_json in
  let docblock =
    opt_field def.docblock "docblock" (fun x -> JSON_String x) in

  JSON_Object ([
    "kind", JSON_String (string_of_kind def.kind);
    "name", JSON_String def.name;
    "id", (opt_string_to_json def.id);
    "position", Pos.json def.pos;
    "span", Pos.multiline_json def.span;
    "modifiers", modifiers;
  ] @
    children @
    params @
    docblock
  )

and outline_response_to_json x =
  Hh_json.JSON_Array (List.map x ~f:definition_to_json)

let coverage_levels_response_to_json = function
  | Range_coverage_levels_response _ -> should_not_happen
  | Deprecated_text_span_coverage_levels_response spans ->
      let opt_coverage_level_to_string = Option.value_map
        ~f:Coverage_level.string_of_level
        ~default:"default"
      in
      let span_to_json (color, text) =
        JSON_Object [
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
  | Some {symbol_name; references} ->
    let entries = List.map references begin fun x ->
      let open Ide_api_types in
      Hh_json.JSON_Object [
        "name", Hh_json.JSON_String symbol_name;
        "filename", Hh_json.JSON_String x.range_filename;
        "line", Hh_json.int_ x.file_range.st.line;
        "char_start", Hh_json.int_ x.file_range.st.column;
        "char_end", Hh_json.int_ (x.file_range.ed.column - 1);
      ]
    end in
    Hh_json.JSON_Array entries

let highlight_references_response_to_json l =
  JSON_Array begin
    List.map l ~f:begin fun x ->
      let open Ide_api_types in
      Hh_json.JSON_Object [
        "line", Hh_json.int_ x.st.line;
        "char_start", Hh_json.int_ x.st.column;
        "char_end", Hh_json.int_ (x.ed.column - 1);
      ]
    end
  end

let diagnostics_to_json x =
  JSON_Object [
    ("filename", JSON_String x.diagnostics_notification_filename);
    ("errors", JSON_Array (List.map x.diagnostics ~f:Errors.to_json));
  ]

let response_to_json = function
  (* Init request is not part of Nuclide protocol *)
  | Init_response _ -> should_not_happen
  | Autocomplete_response x -> autocomplete_response_to_json x
  | Infer_type_response x -> infer_type_response_to_json x
  | Identify_symbol_response x -> identify_symbol_response_to_json x
  | Outline_response x -> outline_response_to_json x
  | Coverage_levels_response x -> coverage_levels_response_to_json x
  | Symbol_by_id_response x -> symbol_by_id_response_to_json x
  | Find_references_response x -> find_references_response_to_json x
  | Highlight_references_response x -> highlight_references_response_to_json x
  | Format_response _ -> should_not_happen

let notification_to_json = function
  | Diagnostics_notification x -> diagnostics_to_json x

let to_json ~message = match message with
  | Response r -> response_to_json r
  | Request (Server_notification n) -> notification_to_json n
  (* There is no use-case for printing client requests for now *)
  | Request (Client_request _) ->  failwith "not implemented"

let print_json ~response =
  to_json (Response response) |>
  Hh_json.json_to_string |>
  print_endline

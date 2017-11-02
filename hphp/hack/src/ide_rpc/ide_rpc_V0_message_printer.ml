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
open Ide_api_types
open Ide_message
open Ide_parser_utils
open Hh_json

let pos_to_json {line; column} =
  JSON_Object [
    ("line", JSON_Number (string_of_int line));
    ("column", JSON_Number (string_of_int column));
  ]

let range_to_json {st; ed} =
  JSON_Object [
    ("start", pos_to_json st);
    ("end", pos_to_json ed);
  ]

let file_range_to_json {range_filename; file_range} =
  JSON_Object [
    ("filename", JSON_String range_filename);
    ("range", range_to_json file_range);
  ]

let init_response_to_json { server_api_version } = JSON_Object [
  ("server_api_version", int_ server_api_version);
]

let autocomplete_response_to_json x =
  let param_to_json x = JSON_Object [
    ("name", JSON_String x.callable_param_name);
    ("type", JSON_String x.callable_param_type);
  ] in

  let callable_details_to_json x = JSON_Object [
    ("return_type", JSON_String x.return_type);
    ("params", JSON_Array (List.map x.callable_params ~f:param_to_json));
  ] in

  let callable_details_to_json = function
    | None -> []
    | Some x -> [("callable_details", callable_details_to_json x)] in

  let autocomplete_response_to_json x = JSON_Object ([
    ("name", JSON_String x.autocomplete_item_text);
    ("type", JSON_String x.autocomplete_item_type);
  ] @ (callable_details_to_json x.callable_details)) in

  JSON_Array (List.map x ~f:autocomplete_response_to_json)

let infer_type_response x = opt_string_to_json x.type_string

let rec symbol_definition_to_json ?include_filename def =
  let open SymbolDefinition in
  let modifiers = JSON_Array (List.map def.modifiers
    ~f:(fun x -> JSON_String (string_of_modifier x))) in
  let children =
    opt_field def.children "children" outline_response_to_json  in
  let params =
    opt_field def.params "params" outline_response_to_json in
  let docblock =
    opt_field def.docblock "docblock" (fun x -> JSON_String x) in

  let filename =
    if Option.value include_filename ~default:false
    then [("filename", JSON_String (Pos.filename def.pos))] else []
  in

  JSON_Object ([
    "name", JSON_String def.name;
    "kind", JSON_String (string_of_kind def.kind);
    "position", def.pos |> pos_to_ide_pos |> pos_to_json;
    "span", def.span |> pos_to_range |> range_to_json;
    "modifiers", modifiers;
  ] @
    filename @
    children @
    params @
    docblock
  )

and outline_response_to_json x =
  JSON_Array (List.map x ~f:symbol_definition_to_json)

let coverage_levels_response_to_json x = match x with
  | Range_coverage_levels_response spans ->
      let span_to_json (range, level) =
        JSON_Object [
          ("level", JSON_String (Coverage_level.string_of_level level));
          ("range", range_to_json range);
        ]
      in
      JSON_Array (List.map spans ~f:span_to_json)
  | deprecated_response ->
      let response = Coverage_levels_response deprecated_response in
      Nuclide_rpc_message_printer.to_json (Response response)

let identify_symbol_response_to_json results =
  JSON_Array (List.map results ~f:begin fun { occurrence; definition } ->
    let definition = Option.value_map definition
      ~f:(symbol_definition_to_json ~include_filename:true)
      ~default:JSON_Null
    in
    let open SymbolOccurrence in
    JSON_Object [
      ("name", JSON_String occurrence.name);
      ("kind", JSON_String (kind_to_string occurrence.type_));
      ("span", occurrence.pos |> pos_to_range |> range_to_json);
      ("definition", definition);
    ]
  end)

let find_references_response_to_json = function
  | None -> JSON_Null
  | Some {symbol_name; references} ->
    let references = List.map references ~f:file_range_to_json in
    JSON_Object [
      ("name", JSON_String symbol_name);
      ("references", JSON_Array references);
    ]

let highlight_references_response_to_json x =
  JSON_Array begin
    List.map x ~f:range_to_json
  end

let format_response_to_json x = JSON_String x

let error_to_json error =
  JSON_Array (List.map (Errors.to_list error) ~f:begin fun (pos, message) ->
    JSON_Object [
      ("message", JSON_String message);
      ("range", pos_to_file_range pos |> file_range_to_json);
    ]
  end)

let diagnostics_to_json x =
  JSON_Object [
    ("filename", JSON_String x.diagnostics_notification_filename);
    ("errors", JSON_Array (List.map x.diagnostics ~f:error_to_json));
  ]

let response_to_json = function
  | Init_response x -> init_response_to_json x
  | Autocomplete_response x -> autocomplete_response_to_json x
  | Infer_type_response x -> infer_type_response x
  | Identify_symbol_response x -> identify_symbol_response_to_json x
  | Outline_response x -> outline_response_to_json x
  | Find_references_response x -> find_references_response_to_json x
  | Highlight_references_response x -> highlight_references_response_to_json x
  | Format_response x -> format_response_to_json x
  | Coverage_levels_response x -> coverage_levels_response_to_json x
  (* Delegate unhandled messages to previous version of API *)
  | r -> Nuclide_rpc_message_printer.to_json (Response r)

let notification_to_json = function
  | Diagnostics_notification x -> diagnostics_to_json x

let to_json ~message = match message with
  | Response r -> response_to_json r
  | Request (Server_notification n) -> notification_to_json n
  | Request _ as m -> Nuclide_rpc_message_printer.to_json m

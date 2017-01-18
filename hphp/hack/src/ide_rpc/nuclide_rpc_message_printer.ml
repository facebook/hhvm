(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Core
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
  let ty_json = match x with
    | Some x -> Hh_json.JSON_String x
    | None -> Hh_json.JSON_Null
  in
  Hh_json.JSON_Object [
    ("type", ty_json);
    ("pos", deprecated_pos_field);
  ]

let identify_symbol_response_to_json results =

  let get_definition_data = function
    | Some x ->
        let open SymbolDefinition in
        let pos = Pos.json x.pos in
        let span = Pos.multiline_json x.span in
        let id = match x.id with
          | Some id -> JSON_String id
          | None -> JSON_Null
        in
        pos, span, id
    | None -> (JSON_Null, JSON_Null, JSON_Null)
  in

  let symbol_to_json { occurrence; definition; } =
    let definition_pos, definition_span, definition_id =
      get_definition_data definition in
    let open SymbolOccurrence in
    JSON_Object [
      "name", JSON_String occurrence.name;
      "result_type",
        JSON_String SymbolOccurrence.(kind_to_string occurrence.type_);
      "pos", Pos.json occurrence.pos;
      "definition_pos", definition_pos;
      "definition_span", definition_span;
      "definition_id", definition_id;
    ]
  in
  JSON_Array (List.map results ~f:symbol_to_json)

let opt_string_to_json = function
  | Some x -> JSON_String x
  | None -> JSON_Null

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

let symbol_by_id_response_to_json = function
  | Some def -> definition_to_json def
  | None -> JSON_Null

let diagnostics_to_json x =
  JSON_Object [
    ("filename", JSON_String x.diagnostics_notification_filename);
    ("errors", JSON_Array (List.map x.diagnostics ~f:Errors.to_json));
  ]

let response_to_json id result =
  let id = match id with
    | Some x -> JSON_Number (string_of_int x)
    | None -> JSON_Null
  in
  JSON_Object [
    ("protocol", JSON_String "service_framework3_rpc");
    ("type", JSON_String "response");
    ("id", id);
    ("result", result);
  ]

let subscription_to_json id result=
   JSON_Object [
    ("protocol", JSON_String "service_framework3_rpc");
    ("type", JSON_String "next");
    ("id", int_ id);
    ("value", result);
  ]

let to_json ~response = match response with
  (* Init request is not part of Nuclide protocol *)
  | Init_response _ -> should_not_happen
  | Autocomplete_response x -> autocomplete_response_to_json x
  | Infer_type_response x -> infer_type_response_to_json x
  | Identify_symbol_response x -> identify_symbol_response_to_json x
  | Outline_response x -> outline_response_to_json x
  | Symbol_by_id_response x -> symbol_by_id_response_to_json x
  | Diagnostics_notification x -> diagnostics_to_json x

let print_json ~response =
  to_json ~response |>
  Hh_json.json_to_string |>
  print_endline

let to_message_json ~id ~response =
  to_json ~response |>
  match response with
  (* Subscriptions are special snowflakes with different output format *)
  | Diagnostics_notification x -> subscription_to_json x.subscription_id
  | _ -> response_to_json id

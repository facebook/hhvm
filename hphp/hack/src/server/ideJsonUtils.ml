(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open IdeJson
open Core
open Hh_json
open Result
open Result.Monad_infix
open Utils

let server_busy_error_code = 1
let invalid_call_error_code = 2

let string_positions_to_content_pos line column =
  try
    {
      line = int_of_string line;
      column = int_of_string column
    }
  with Failure "int_of_string" -> raise Not_found

(**
 * For persistent connection we read json bundle from client/editor, so an auto
 * -complete call may looks loke:
 *
 * {
 *   "protocol" : "name_of_protocol"
 *   "id" = <some_number>,
 *   "type" = "call",
 *   "method" = "auto_complete"
 *   "args" = {
 *       "filename" : "somename.php",
 *       "position" : {
 *          "line" : <some_number>,
 *          "column" : <some_number>}
 *       }
 * }
 *
 * This function translates those command and args into a call_type structure.
*)
let to_call cmd args =
  let get_field fields field_name =
    match List.find fields (fun (x, _) -> x = field_name) with
    | Some (_, x) -> Some x
    | None -> None in

  let get_filename () =
    match get_field args "filename" with
    | Some (JSON_String path) -> path
    | _ -> raise Not_found in

  let parse_position pos =
    let line = match get_field pos "line" with
      | Some (JSON_Number line) -> line
      | _ -> raise Not_found in
    let column = match get_field pos "column" with
      | Some (JSON_Number column) -> column
      | _ -> raise Not_found in
    string_positions_to_content_pos line column in

  let parse_span span =
    let st = match get_field span "start" with
      | Some (JSON_Object st) -> st
      | _ -> raise Not_found in
    let st = parse_position st in
    let ed = match get_field span "end" with
      | Some (JSON_Object ed) -> ed
      | _ -> raise Not_found in
    let ed = parse_position ed in
    st, ed in

  let parse_edit edit =
    let span = match get_field edit "span" with
      | Some (JSON_Object span) -> span
      | _ -> raise Not_found in
    let st, ed = parse_span span in
    let text = match get_field edit "text" with
      | Some (JSON_String text) -> text
      | _ -> raise Not_found in
    {st; ed; text} in

  let parse_edits : json list -> code_edit list = fun edits ->
    List.fold ~init:[] ~f:(fun l e ->
      let edit = match e with
      | JSON_Object edit -> edit
      | _ -> raise Not_found in
      l @ [parse_edit edit]) edits in

  match cmd with
  | "getCompletions" ->
    let path = get_filename () in
    let pos = match get_field args "position" with
      | Some (JSON_Object pos) -> pos
      | _ -> raise Not_found in
    let pos = parse_position pos in
    Auto_complete_call (path, pos)
  | "open" ->
    Open_file_call (get_filename ())
  | "close" ->
    Close_file_call (get_filename ())
  | "edit" ->
    let path = get_filename () in
    let edits = match get_field args "edits" with
      | Some (JSON_Array edits) -> edits
      | _ -> raise Not_found in
    let edits = parse_edits edits in
    Edit_file_call (path, edits)
  | "disconnect" ->
    Disconnect_call
  | _ -> raise Not_found

let call_of_string s =
  let get_object_fields s =
    try
      begin match json_of_string s with
        | JSON_Object fields -> Ok fields
        | _ ->  Error `Not_object
      end
    with Syntax_error e -> Error (`Syntax_error e) in

  let get_field fields field_name  =
    match List.find fields (fun (x, _) -> x = field_name) with
    | Some (_, x) -> Some x
    | None -> None in

  let check_protocol_field fields =
    match get_field fields "protocol" with
    | Some protocol -> begin match protocol with
      | JSON_String "service_framework3_rpc" -> Ok ()
      | JSON_String _ -> Error `Unknown_protocol
      | _ -> Error `Protocol_not_string
    end
    | None -> Error `No_protocol in

  let get_id_field fields =
    match get_field fields "id" with
    | Some id -> begin match id with
      | JSON_Number i ->
        (try Ok (int_of_string i) with Failure _ -> Error `Id_not_int)
      | _ ->  Error `Id_not_int
    end
    | None -> Error `No_id in

  let get_type_field fields =
    match get_field fields "type" with
    | Some t -> begin match t with
      | JSON_String "call" -> Ok "call"
      | JSON_String _ -> Error `Message_type_not_recognized
      | _ -> Error `Message_type_not_string
    end
    | None -> Error `No_type in

  let get_method_field fields =
    match get_field fields "method" with
    | Some cmd -> begin match cmd with
      | JSON_String c -> Ok c
      | _ ->  Error `Method_not_string
    end
    | None -> Error `No_method in

  let get_call id cmd fields =
    match get_field fields "args" with
    | Some (JSON_Object args) ->
      begin
        try
          Ok (Call (id, to_call cmd args))
        with Not_found -> Error (`Call_not_recognized id)
      end
    | Some _ -> Error (`Args_not_an_object id)
    | _ -> Error (`No_args id) in

  match
    (get_object_fields s) >>= fun fields ->
    (check_protocol_field fields) >>= fun _ ->
    (get_id_field fields) >>= fun id ->
    (get_type_field fields) >>= fun _type ->
    (get_method_field fields) >>= fun cmd ->
    (get_call id cmd fields)
  with
  | Ok x -> x
  | Error `Syntax_error e -> Parsing_error ("Invalid JSON: " ^ e)
  | Error `Not_object -> Parsing_error "Expected JSON object"
  | Error `No_protocol ->
    Parsing_error "Request object must have protocol field"
  | Error `Protocol_not_string ->
    Parsing_error "Protocol field must be a string"
  | Error `Unknown_protocol -> Parsing_error "Unknown protocol"
  | Error `No_id -> Parsing_error "Request object must have id field"
  | Error `Id_not_int -> Parsing_error "id field must be an integer"
  | Error `No_method -> Parsing_error "Request object must have method field"
  | Error `Method_not_string -> Parsing_error "Method field must be a string"
  | Error `No_type -> Parsing_error "Request object must have type field"
  | Error `Message_type_not_string ->
    Parsing_error "Type field must be a string"
  | Error `Message_type_not_recognized ->
    Parsing_error "Message type not recognized"
  | Error `No_args id ->
    Invalid_call (id, "Request object must have an args field")
  | Error `Args_not_an_object id ->
    Invalid_call (id, "Args field must be an object")
  | Error `Call_not_recognized id -> Invalid_call (id, "Call not recognized")

let build_response_json id result_field =
  JSON_Object [
    ("type", JSON_String "response");
    ("id", JSON_Number (string_of_int id));
    ("result", result_field);
  ]

let json_string_of_response id response =
  let result_field = match response with
    | Auto_complete_response r -> r
    | Identify_function_response s -> JSON_String s
    | Search_call_response r -> r
    | Status_response r -> r
    | Find_refs_response r -> ServerFindRefs.to_json r
    | Colour_response r -> r
    | Find_lvar_refs_response pos_list ->
      let res_list = List.map pos_list (compose Pos.json Pos.to_absolute) in
      JSON_Object [
        "positions",      JSON_Array res_list;
        "internal_error", JSON_Bool false
      ]
    | Type_at_pos_response (pos, ty) -> ServerInferType.to_json pos ty
    | Format_response result ->
      let error_text, result, is_error = match result with
        | Format_hack.Disabled_mode -> "Php_or_decl", "", false
        | Format_hack.Parsing_error _ -> "Parsing_error", "", false
        | Format_hack.Internal_error -> "", "", true
        | Format_hack.Success s -> "", s, false
      in
      JSON_Object [
        "error_message",  JSON_String error_text;
        "result",         JSON_String result;
        "internal_error", JSON_Bool is_error;
      ]
    | Get_method_name_response result ->
      begin match result with
      | Some res ->
        let result_type =
          match res.SymbolOccurrence.type_ with
          | SymbolOccurrence.Class -> "class"
          | SymbolOccurrence.Method _ -> "method"
          | SymbolOccurrence.Function -> "function"
          | SymbolOccurrence.LocalVar -> "local"
          | SymbolOccurrence.Property _ -> "property"
          | SymbolOccurrence.ClassConst _ -> "class_const"
          | SymbolOccurrence.Typeconst _ -> "typeconst"
          | SymbolOccurrence.GConst -> "global_const"
        in
        JSON_Object [
          "name",        JSON_String res.SymbolOccurrence.name;
          "result_type", JSON_String result_type;
          "pos",         Pos.json (Pos.to_absolute res.SymbolOccurrence.pos);
        ]
      | _ -> JSON_Object []
      end
    | Outline_response result ->
      FileOutline.to_json_legacy result
  in
  json_to_string (build_response_json id result_field)

let json_string_of_error id error_code error_message  =
  json_to_string (JSON_Object [
    ("type", JSON_String "response");
    ("id", JSON_Number (string_of_int id));
    ("error", JSON_Object [
       ("code", JSON_Number (string_of_int error_code));
       ("message", JSON_String error_message);
     ]);
  ])

let json_string_of_invalid_call id error_message =
  json_string_of_error id invalid_call_error_code error_message

let json_string_of_server_busy id =
  json_string_of_error id server_busy_error_code "Server busy"

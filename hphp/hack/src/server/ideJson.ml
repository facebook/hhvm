(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Every message has a unique id, for debugging and pairing requests with
 * responses. *)

type call_id = int

type call_type =
  | Auto_complete_call of string * File_content.content_pos
  | Highlight_ref_call of string * File_content.content_pos
  | Identify_function_call of string * File_content.content_pos
  | Open_file_call of string
  | Close_file_call of string
  | Edit_file_call of string * (File_content.code_edit list)
  | Disconnect_call
  | Subscribe_diagnostic_call
  | Unsubscribe_diagnostic_call
  | Sleep_for_test

type diagnostic_response = {
  path : string;
  diagnostics : Pos.absolute Errors.error_ list;
}

type response_type =
  | Auto_complete_response of Hh_json.json
  | Idetify_function_response of Hh_json.json
  | Diagnostic_response of call_id * diagnostic_response
  | Highlight_ref_response of Hh_json.json

type parsing_result =
  (* Parsing_error means that message was unrecoverably mangled (eg. no ID, or
   * completely invalid JSON). We will just log it, but not send anything back
   * to the client. *)
  | Parsing_error of string
  (* Invalid_call will get an error response from the server. *)
  | Invalid_call of call_id * string
  | Call of call_id * call_type

let to_string call =
  match call with
  | Auto_complete_call _ -> "getCompletions"
  | Highlight_ref_call _ -> "getSourceHighlights"
  | Identify_function_call _ -> "getDefinition"
  | Open_file_call _ -> "didOpenFile"
  | Close_file_call _ -> "didCloseFile"
  | Edit_file_call _ -> "didChangeFile"
  | Disconnect_call -> "disconnect"
  | Subscribe_diagnostic_call -> "notifyDiagnostics"
  | Unsubscribe_diagnostic_call -> "unsubscribe"
  | Sleep_for_test -> "sleep"

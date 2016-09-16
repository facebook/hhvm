(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hh_json
open IdeJson
open IdeJsonUtils
open File_content

let test_invalid_json () =
  let msg = "aaaa" in
  match call_of_string msg with
  | Parsing_error _ -> true
  | _ -> false

let expect_parsing_error input error =
  match call_of_string input with
  | Parsing_error x when x = error -> true
  | _ -> false

let expect_invalid_call input id error =
  match call_of_string input with
  | Invalid_call (x, y) when x = id && y = error -> true
  | _ -> false

let test_non_object () =
  expect_parsing_error
    "[]"
    "Expected JSON object"

let test_no_protocol () =
  expect_parsing_error
    "{}"
    "Request object must have protocol field"

let test_unkown_protocol () =
  expect_parsing_error
    {|{"protocol" : "aaa"}|}
    "Unknown protocol"

let test_protocol_not_string () =
  expect_parsing_error
    {|{"protocol" : 123}|}
    "Protocol field must be a string"

let test_no_call_id () =
  expect_parsing_error
    {|{"protocol" : "service_framework3_rpc"}|}
    "Request object must have id field"

let test_call_id_not_int () =
  expect_parsing_error
    {|{"protocol" : "service_framework3_rpc", "id" : "aaa"}|}
    "id field must be an integer"

let test_call_id_not_int2 () =
  expect_parsing_error
    {|{"protocol" : "service_framework3_rpc", "id" : 12.3}|}
    "id field must be an integer"

let test_no_type () =
  expect_parsing_error
    {|{"protocol" : "service_framework3_rpc", "id" : 4}|}
    "Request object must have type field"

let test_type_not_string () =
  expect_parsing_error
    {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : 4}|}
    "Type field must be a string"

let test_type_not_recognized () =
  expect_parsing_error
    {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "aaaa"}|}
    "Message type not recognized"

let test_no_method () =
  expect_parsing_error
    {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "call"}|}
    "Request object must have method field"

let test_method_not_string () =
  expect_parsing_error
    {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "call",
      "method" : 4}|}
    "Method field must be a string"

let test_no_args () =
  expect_invalid_call
    {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "call",
      "method" : "open"}|}
    4
    "Request object must have an args field"

let test_args_not_object () =
  expect_invalid_call
    {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "call",
      "method" : "open", "args" : 4}|}
    4
    "Args field must be an object"

let test_call_not_recognized () =
  expect_invalid_call
    {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "call",
      "method" : "no_such_command", "args" : {}}|}
    4
    "Call not recognized"

let test_call_id = 4

let build_position_msg x y =
  {|{"line" : |} ^ x ^ {|, "column" : |} ^ y ^ "}"

let build_range_msg span =
  match span with
  | Some (st, ed) ->
    {|"range" : {"start" : |} ^ st ^ {|, "end" : |} ^ ed ^ {|}, |}
  | None ->
    ""

let build_edit_msg l =
  let edits = List.map (fun (span, text) -> "{" ^ (build_range_msg span) ^
    {|"text" : |} ^ text ^ "}") l in
  let edits = String.concat ", " edits in
  {|"changes" : [|} ^ edits ^ "]"

let build_call_msg cmd args =
  let args = String.concat ", " args in
  {|{"protocol" : "service_framework3_rpc", "id" : |} ^
    string_of_int test_call_id ^ {|, "type" : "call", "method" : |} ^ cmd ^
    {|,"args" : {|} ^ args ^ "}}"

let expect_call msg call =
  match call_of_string msg with
  | Call (x, y) when x = test_call_id && y = call -> true
  | _ -> false

let test_autocomplete_call () =
  let msg = build_call_msg "\"getCompletions\"" [
    {|"filename" : "test.php"|};
    {|"position" : |} ^ (build_position_msg "9" "10")
  ] in
  expect_call msg (Auto_complete_call ("test.php", {line = 9; column = 10}))

let test_autocomplete_response () =
  let id = 4 in
  let response = Auto_complete_response (JSON_Array []) in
  (json_string_of_response id response) =
  {|{"protocol":"service_framework3_rpc","type":"response","id":4,"result":[]}|}

let test_invalid_call_response () =
  let id = 4 in
  let msg = "error" in
  (json_string_of_invalid_call id msg) =
  {|{"type":"response","id":4,"error":{"code":2,"message":"error"}}|}

let test_server_busy_reponse () =
  let id = 4 in
  (json_string_of_server_busy id) =
  {|{"type":"response","id":4,"error":{"code":1,"message":"Server busy"}}|}

let test_open_file_call () =
  let msg = build_call_msg "\"didOpenFile\"" [
    {|"filename" : "test.php"|};
  ] in
  expect_call msg (Open_file_call "test.php")

let test_close_file_call () =
  let msg = build_call_msg "\"didCloseFile\"" [
    {|"filename" : "test.php"|};
  ] in
  expect_call msg (Close_file_call "test.php")

let test_edit_file_call () =
  let msg = build_call_msg "\"didChangeFile\"" [
    {|"filename" : "test.php"|};
    build_edit_msg [
    Some ((build_position_msg "0" "0"), (build_position_msg "0" "2")),
    {|"<?hh"|};
    Some ((build_position_msg "2" "2"), (build_position_msg "2" "4")),
    {|"class"|};];
  ] in
  expect_call msg (Edit_file_call
    ("test.php",
    [{range = Some {
        st = {line = 0; column = 0};
        ed = {line = 0; column = 2}};
     text = "<?hh"};
     {range = Some {
        st = {line = 2; column = 2};
        ed = {line = 2; column = 4}};
      text = "class"};]))

let test_edit_file_call2 () =
  let msg = build_call_msg "\"didChangeFile\"" [
    {|"filename" : "test.php"|};
    build_edit_msg [
    None,
    {|"<?hh"|};
    Some ((build_position_msg "2" "2"), (build_position_msg "2" "4")),
    {|"class"|};];
  ] in
  expect_call msg (Edit_file_call
    ("test.php",
    [{range = None;
     text = "<?hh"};
     {range = Some {
        st = {line = 2; column = 2};
        ed = {line = 2; column = 4}};
      text = "class"};]))

let test_disconnect_call () =
  let msg = build_call_msg "\"disconnect\"" [] in
  expect_call msg (Disconnect_call)

let test_subscribe_call () =
  let msg = build_call_msg "\"notifyDiagnostics\"" [] in
  expect_call msg (Subscribe_diagnostic_call)

let test_diagnostic_response () =
  let id = 4 in
  let diagnostics = { path = "foo.php"; diagnostics = [] } in
  let response = Diagnostic_response (id, diagnostics) in
  (json_string_of_response id response) =
  {|{"protocol":"service_framework3_rpc","type":"next","id":4,|} ^
  {|"diagnostics":{"filename":"foo.php","errors":[]}}|}

let test_unsubscribe_call () =
  let msg =
    {|{"protocol":"service_framework3_rpc","id":4,"type":"unsubscribe"}|} in
  expect_call msg (Unsubscribe_diagnostic_call)

let test_highlight_ref_call () =
  let msg = build_call_msg "\"getSourceHighlights\"" [
    {|"filename" : "test.php"|};
    {|"position" : |} ^ (build_position_msg "2" "3")
  ] in
  expect_call msg (Highlight_ref_call ("test.php", {line = 2; column = 3}))

let test_highlight_ref_response () =
  let id = 4 in
  let response = Highlight_ref_response (JSON_Array []) in
  (json_string_of_response id response) =
  {|{"protocol":"service_framework3_rpc","type":"response","id":4,"result":[]}|}

let test_identify_funtion_call () =
  let msg = build_call_msg "\"getDefinition\"" [
    {|"filename" : "test.php"|};
    {|"position" : |} ^ (build_position_msg "2" "3")
  ] in
  expect_call msg (Identify_function_call ("test.php", {line = 2; column = 3}))

let test_identify_funtion_response () =
  let id = 4 in
  let response = Idetify_function_response (JSON_Array []) in
  (json_string_of_response id response) =
  {|{"protocol":"service_framework3_rpc","type":"response","id":4,"result":[]}|}

let tests = [
  "test_invalid_json", test_invalid_json;
  "test_non_object", test_non_object;
  "test_no_protocol", test_no_protocol;
  "test_unkown_protocol", test_unkown_protocol;
  "test_protocol_not_string", test_protocol_not_string;
  "test_no_call_id", test_no_call_id;
  "test_call_id_not_int", test_call_id_not_int;
  "test_call_id_not_int2", test_call_id_not_int2;
  "test_no_type", test_no_type;
  "test_type_not_string", test_type_not_string;
  "test_type_not_recognized", test_type_not_recognized;
  "test_no_method", test_no_method;
  "test_method_not_string", test_method_not_string;
  "test_no_args", test_no_args;
  "test_args_not_object", test_args_not_object;
  "test_call_not_recognized", test_call_not_recognized;
  "test_autocomplete_call", test_autocomplete_call;
  "test_autocomplete_response", test_autocomplete_response;
  "test_open_file_call", test_open_file_call;
  "test_close_file_call", test_close_file_call;
  "test_edit_file_call", test_edit_file_call;
  "test_edit_file_call2", test_edit_file_call2;
  "test_disconnect_call", test_disconnect_call;
  "test_subscribe_call", test_subscribe_call;
  "test_diagnostic_response", test_diagnostic_response;
  "test_unsubscribe_call", test_unsubscribe_call;
  "test_highlight_ref_call", test_highlight_ref_call;
  "test_highlight_ref_response", test_highlight_ref_response;
  "test_identify_funtion_call", test_identify_funtion_call;
  "test_identify_funtion_response", test_identify_funtion_response;
]

let () =
  Unit_test.run_all tests

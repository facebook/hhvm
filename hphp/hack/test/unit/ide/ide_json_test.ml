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

let test_invalid_json () =
  let msg = "aaaa" in
  match call_of_string msg with
  | Parsing_error _ -> true
  | _ -> false

let expect_parsing_error input error =
  match call_of_string input with
  | Parsing_error error -> true
  | _ -> false

let expect_invalid_call input id error =
  match call_of_string input with
  | Invalid_call (id, error) -> true
  | _ -> false

let test_non_object () =
  expect_parsing_error
    "[]"
    "Expected JSON object"

let test_no_call_id () =
  expect_parsing_error
    "{}"
    "Request object must have id field"

let test_call_id_not_int () =
  expect_parsing_error
    {|{"id" : "aaa"}|}
    "id field must be an integer"

let test_call_id_not_int2 () =
  expect_parsing_error
    {|{"id" : 12.3}|}
    "id field must be an integer"

let test_no_type () =
  expect_parsing_error
    {|{"id" : 4}|}
    "Request object must have type field"

let test_type_not_string () =
  expect_parsing_error
    {|{"id" : 4, "type" : 4}|}
    "Type field must be string"

let test_type_not_recognized () =
  expect_parsing_error
    {|{"id" : 4, "type" : "aaaa"}|}
    "Message type not recognized"

let test_no_args () =
  expect_invalid_call
    {|{"id" : 4, "type" : "call"}|}
    4
    "Request object must have an args field"

let test_args_not_array () =
  expect_invalid_call
    {|{"id" : 4, "type" : "call", "args" : 4}|}
    4
    "Args field must be an array"

let test_call_not_recognized () =
  expect_invalid_call
    {|{"id" : 4, "type" : "call", "args" : ["no_such_command"]}|}
    4
    "Call not recognized"

let test_call_id = 4

let build_call_msg args =
  let args = String.concat ", " args in
  {|{"id" : |} ^ string_of_int test_call_id ^
    {|, "type" : "call", "args" : [|} ^ args ^ "]}"

let expect_call msg call =
  match call_of_string msg with
  | Call (test_call_id, call) -> true
  | _ -> false

let test_autocomplete_call () =
  let msg = build_call_msg [
    {|"--auto-complete"|};
    {|"<?hh"|}
  ] in
  expect_call msg (Auto_complete_call ("<?hh"))

let test_autocomplete_response () =
  let id = 4 in
  let response = Auto_complete_response (JSON_Array []) in
  (json_string_of_response id response) =
  {|{"type":"response","id":4,"result":[]}|}

let test_invalid_call_response () =
  let id = 4 in
  let msg = "error" in
  (json_string_of_invalid_call id msg) =
  {|{"type":"response","id":4,"error":{"code":2,"message":"error"}}|}

let test_server_busy_reponse () =
  let id = 4 in
  (json_string_of_server_busy id) =
  {|{"type":"response","id":4,"error":{"code":1,"message":"Server busy"}}|}

let test_identify_function_call () =
  let msg = build_call_msg [
    {|"--identify-function"|};
    {|"21:37"|};
    {|"<?hh"|}
  ] in
  expect_call msg (Identify_function_call ("<?hh", 21, 37))

let test_status_call () =
  let msg = build_call_msg [] in
  expect_call msg Status_call

let test_find_function_refs_call () =
  let msg = build_call_msg [
    {|"--find-refs"|};
    {|"array_pull"|};
  ] in
  expect_call msg (Find_refs_call (FindRefsService.Function "array_pull"))

let test_find_method_refs_call () =
  let msg = build_call_msg [
    {|"--find-refs"|};
    {|"C::getID"|}
  ] in
  expect_call msg (Find_refs_call (FindRefsService.Method ("C", "getID")))

let test_find_class_refs_call () =
  let msg = build_call_msg [
    {|"--find-class-refs"|};
    {|"C"|}
  ] in
  expect_call msg (Find_refs_call (FindRefsService.Class "C"))

let test_strip_json_arg () =
  let msg = build_call_msg [
    {|"--json"|};
    {|"--identify-function"|};
    {|"21:37"|};
    {|"<?hh"|}
  ] in
  expect_call msg (Identify_function_call ("<?hh", 21, 37))

let tests = [
  "test_invalid_json", test_invalid_json;
  "test_non_object", test_non_object;
  "test_no_call_id", test_no_call_id;
  "test_call_id_not_int", test_call_id_not_int;
  "test_call_id_not_int2", test_call_id_not_int2;
  "test_no_type", test_no_type;
  "test_type_not_string", test_type_not_string;
  "test_type_not_recognized", test_type_not_recognized;
  "test_no_args", test_no_args;
  "test_args_not_array", test_args_not_array;
  "test_call_not_recognized", test_call_not_recognized;
  "test_autocomplete_call", test_autocomplete_call;
  "test_autocomplete_response", test_autocomplete_response;
  "test_invalid_call_response", test_invalid_call_response;
  "test_server_busy_reponse", test_server_busy_reponse;
  "test_identify_function_call", test_identify_function_call;
  "test_strip_json_arg", test_strip_json_arg;
  "test_status_call", test_status_call;
  "test_find_function_refs_call", test_find_function_refs_call;
  "test_find_method_refs_call", test_find_method_refs_call;
  "test_find_class_refs_call", test_find_class_refs_call;
]

let () =
  Unit_test.run_all tests

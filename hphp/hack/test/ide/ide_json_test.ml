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
  | ParsingError _ -> true
  | _ -> false

let expect_parsing_error input error =
  match call_of_string input with
  | ParsingError error -> true
  | _ -> false

let expect_invalid_call input id error =
  match call_of_string input with
  | InvalidCall (id, error) -> true
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
    "{\"id\" : \"aaa\"}"
    "id field must be an integer"

let test_call_id_not_int2 () =
  expect_parsing_error
    "{\"id\" : 12.3}"
    "id field must be an integer"

let test_no_type () =
  expect_parsing_error
    "{\"id\" : 4}"
    "Request object must have type field"

let test_type_not_string () =
  expect_parsing_error
    "{\"id\" : 4, \"type\" : 4}"
    "Type field must be string"

let test_type_not_recognized () =
  expect_parsing_error
    "{\"id\" : 4, \"type\" : \"aaaa\"}"
    "Message type not recognized"

let test_no_args () =
  expect_invalid_call
    "{\"id\" : 4, \"type\" : \"call\"}"
    4
    "Request object must have an args field"

let test_args_not_array () =
  expect_invalid_call
    "{\"id\" : 4, \"type\" : \"call\", \"args\" : 4}"
    4
    "Args field must be an array"

let test_call_not_recognized () =
  expect_invalid_call
    "{\"id\" : 4, \"type\" : \"call\", \"args\" : []}"
    4
    "Call not recognized"

let test_autocomplete_call () =
  let msg = "{\"id\" : 4, \"type\" : \"call\",  \
             \"args\" : [\"--auto-complete\", \"<?hh\"]}" in
  match call_of_string msg with
  | Call (4, AutoCompleteCall ("<?hh")) -> true
  | _ -> false

let test_autocomplete_response () =
  let id = 4 in
  let response = AutoCompleteResponse (JSON_Array []) in
  (json_string_of_response id response) =
  "{\"type\":\"response\",\"id\":4,\"result\":[]}"

let test_invalid_call_response () =
  let id = 4 in
  let msg = "error" in
  (json_string_of_invalid_call id msg) =
  "{\"type\":\"response\",\"id\":4,\"error\":{\"code\":2,\"message\":\"error\"}}"

let test_server_busy_reponse () =
  let id = 4 in
  (json_string_of_server_busy id) =
  "{\"type\":\"response\",\"id\":4,\"error\":{\"code\":1,\"message\":\"Server busy\"}}"


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
]

let () =
  Unit_test.run_all tests

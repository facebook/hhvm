(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

let test_escape_unescape_data = [
  "newline\n";
  "\"quoted string\"";
  "tab\t";
  "carriage return\r";
  "backslash\\";
  "magic char" ^ (String.make 1 (Char.chr 8));
  "magic_char_with_hexadecimal_digit" ^ (String.make 1 (Char.chr 26));
]

let test_escape_unescape () =
  List.for_all test_escape_unescape_data begin fun s ->
    let json = Hh_json.JSON_String s in
    let encoded = Hh_json.json_to_string json in
    let decoded = Hh_json.json_of_string encoded in
    let result = Hh_json.get_string_exn decoded in
    (result = s)
  end

let test_empty_string () =
  try
    ignore (Hh_json.json_of_string "");
    false
  with Hh_json.Syntax_error _ -> true

let test_whitespace_string () =
  (match Hh_json.json_of_string "\" \"" with
  | Hh_json.JSON_String " " -> true
  | _ -> false)

let test_access_object_string () =
  let json = Hh_json.json_of_string
    "{ \"foo\": { \"bar\": { \"baz\": \"hello\" } } }" in
  let open Hh_json.Access in
  let result = (return json)
    >>= get_obj "foo"
    >>= get_obj "bar"
    >>= get_string "baz"
  in
  match result with
  | Result.Ok ("hello", _) ->
    true
  | _ ->
    false

let test_access_object_bool () =
  let json = Hh_json.json_of_string
    "{ \"foo\": { \"bar\": { \"baz\": true } } }" in
  let open Hh_json.Access in
  let result = (return json)
    >>= get_obj "foo"
    >>= get_obj "bar"
    >>= get_bool "baz"
  in
  match result with
  | Result.Ok (true, _) ->
    true
  | _ ->
    false

let test_access_object_number () =
  let json = Hh_json.json_of_string
    "{ \"foo\": { \"bar\": { \"baz\": 5 } } }" in
  let open Hh_json.Access in
  let result = (return json)
    >>= get_obj "foo"
    >>= get_obj "bar"
    >>= get_number "baz"
  in
  match result with
  | Result.Ok ("5", _) ->
    true
  | _ ->
    false

let test_access_object_val () =
  let json = Hh_json.json_of_string
    "{ \"foo\": { \"bar\": { \"baz\": 5 } } }" in
  let open Hh_json.Access in
  let result = (return json)
    >>= get_obj "foo"
    >>= get_obj "bar"
    >>= get_val "baz"
  in
  match result with
  | Result.Ok (Hh_json.JSON_Number "5", _) ->
    true
  | _ ->
    false

let test_access_object_key_doesnt_exist () =
  let json = Hh_json.json_of_string
    "{ \"foo\": { \"bar\": { \"baz\": 5 } } }" in
  let open Hh_json.Access in
  let result = (return json)
    >>= get_obj "foo"
    >>= get_obj "bar"
    >>= get_number "oops"
  in
  match result with
  | Result.Error (Missing_key_error("oops", ["bar"; "foo"])) ->
    true
  | _ ->
    false

let test_access_object_type_invalid () =
  let json = Hh_json.json_of_string
    "{ \"foo\": { \"bar\": { \"baz\": 5 } } }" in
  let open Hh_json.Access in
  let result = (return json)
    >>= get_obj "foo"
    >>= get_obj "bar"
    >>= get_string "baz"
  in
  match result with
  | Result.Error (Wrong_type_error(["baz"; "bar"; "foo"], Hh_json.String_t)) ->
    true
  | _ ->
    false

(** Hit an error when accessing the third key, in this JSON object
  * of depth 4. *)
let test_access_object_error_in_middle () =
  let json = Hh_json.json_of_string
    "{ \"foo\": { \"bar\": { \"baz\": { \"qux\" : 5 } } } }" in
  let open Hh_json.Access in
  let result = (return json)
    >>= get_obj "foo"
    >>= get_obj "bar"
    >>= get_obj "oops"
    >>= get_obj "baz"
    >>= get_number "qux"
  in
  match result with
  | Result.Error (Missing_key_error ("oops", ["bar"; "foo"])) ->
    true
  | _ ->
    false

let tests = [
  "test_escape_unescape", test_escape_unescape;
  "test_empty_string", test_empty_string;
  "test_whitespace_string", test_whitespace_string;
  "test_access_object_string", test_access_object_string;
  "test_access_object_bool", test_access_object_bool;
  "test_access_object_number", test_access_object_number;
  "test_access_object_val", test_access_object_val;
  "test_access_object_key_doesnt_exit", test_access_object_key_doesnt_exist;
  "test_access_object_type_invalid", test_access_object_type_invalid;
  "test_access_object_error_in_middle", test_access_object_error_in_middle;
]

let () =
  Unit_test.run_all tests

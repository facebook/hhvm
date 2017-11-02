(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

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
  | Ok ("hello", _) ->
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
  | Ok (true, _) ->
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
  | Ok ("5", _) ->
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
  | Ok (Hh_json.JSON_Number "5", _) ->
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
  | Error (Missing_key_error("oops", ["bar"; "foo"])) ->
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
  | Error (Wrong_type_error(["baz"; "bar"; "foo"], Hh_json.String_t)) ->
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
  | Error (Missing_key_error ("oops", ["bar"; "foo"])) ->
    true
  | _ ->
    false

type fbz_record = {
  foo : bool;
  bar : string;
  baz : int;
}

let test_access_3_keys_one_object () =
  let json = Hh_json.json_of_string (
    "{\n"^
    "  \"foo\" : true,\n"^
    "  \"bar\" : \"hello\",\n"^
    "  \"baz\" : 5\n"^
    "}"
  ) in
  let open Hh_json.Access in
  let accessor = return json in
  let result =
    accessor >>= get_bool "foo" >>= fun (foo, _) ->
    accessor >>= get_string "bar" >>= fun (bar, _) ->
    accessor >>= get_number_int "baz" >>= fun (baz, _) ->
    return {
      foo;
      bar;
      baz;
    }
  in
  match result with
  | Error access_failure ->
    Printf.eprintf "Error failed to parse. See: %s\n"
      (access_failure_to_string access_failure);
    false
  | Ok (v, _) ->
    Asserter.Bool_asserter.assert_equals v.foo true "foo value mismatch";
    Asserter.String_asserter.assert_equals v.bar "hello" "bar value mismatch";
    Asserter.Int_asserter.assert_equals v.baz 5 "baz value mismatch";
    true

(** We access exactly as we do above, but "bar" actually is an array instead
 * of a string, so we should expect to get a Error. *)
let test_access_3_keys_one_object_wrong_type_middle () =
  let json = Hh_json.json_of_string (
    "{\n"^
    "  \"foo\" : true,\n"^
    "  \"bar\" : [],\n"^
    "  \"baz\" : 5\n"^
    "}"
  ) in
  let open Hh_json.Access in
  let accessor = return json in
  let result =
    accessor >>= get_bool "foo" >>= fun (foo, _) ->
    accessor >>= get_string "bar" >>= fun (bar, _) ->
    accessor >>= get_number_int "baz" >>= fun (baz, _) ->
    return {
      foo;
      bar;
      baz;
    }
  in
  match result with
  | Error access_failure ->
    Asserter.String_asserter.assert_equals
      "Value expected to be String (at field [bar])"
      (access_failure_to_string access_failure)
      "Not the access failure we expected";
    true
  | Ok (v, _) ->
    Printf.eprintf "Expected failure, but successfully traversed json.\n";
    false

let test_truncate () =
  let s = {|{ "a":{"a1":{"a1x":"hello","a1y":42},"a2":true},"b":null}|} in

  let actual = Hh_json.json_truncate_string s in
  let exp = s in (* we expect it to preserve the leading space! *)
  Asserter.String_asserter.assert_equals exp actual "unchanged truncate";

  let actual = Hh_json.json_truncate_string s ~max_string_length:1 in
  let exp = {|{"a":{"a1":{"a1x":"h...","a1y":42},"a2":true},"b":null}|} in
  Asserter.String_asserter.assert_equals exp actual "max_string_length truncate";

  let actual = Hh_json.json_truncate_string s ~max_child_count:1 in
  let exp = {|{"a":{"a1":{"a1x":"hello"}}}|} in
  Asserter.String_asserter.assert_equals exp actual "max_child_count truncate";

  let actual = Hh_json.json_truncate_string s ~max_depth:1 in
  let exp = {|{"a":{},"b":null}|} in
  Asserter.String_asserter.assert_equals exp actual "max_depth truncate";

  let actual = Hh_json.json_truncate_string s ~max_total_count:1 in
  let exp = {|{"a":{}}|} in
  Asserter.String_asserter.assert_equals exp actual "max_total_count truncate 1";

  let actual = Hh_json.json_truncate_string s ~max_total_count:2 in
  let exp = {|{"a":{"a1":{}}}|} in
  Asserter.String_asserter.assert_equals exp actual "max_total_count truncate 2";
  true

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
  "test_access_3_keys_on_object", test_access_3_keys_one_object;
  "test_access_3_keys_one_object_wrong_type_middle",
    test_access_3_keys_one_object_wrong_type_middle;
  "test_truncate", test_truncate;
]

let () =
  Unit_test.run_all tests

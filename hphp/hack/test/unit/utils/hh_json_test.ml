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

let tests = [
  "test_escape_unescape", test_escape_unescape;
  "test_empty_string", test_empty_string;
  "test_whitespace_string", test_whitespace_string
]

let () =
  Unit_test.run_all tests

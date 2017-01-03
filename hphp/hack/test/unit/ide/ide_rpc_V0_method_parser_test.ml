(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* Test suite for V0 version of the API methods *)

open File_content
open Ide_message
open Ide_message_parser_test_utils

let build_request_message = Printf.sprintf
  {|{
    "jsonrpc" : "2.0",
    "id" : 4,
    "method" : "%s" ,
    "params" : {%s}
  }|}

let test_did_open_file () =
  let msg = build_request_message "didOpenFile"
    {|
      "filename" : "test.php",
      "text" : "<?hh"
    |} in
  test msg @@
  expect_api_message (
    Did_open_file {
      did_open_file_filename = "test.php";
      did_open_file_text =  "<?hh";
    }
  )

let test_autocomplete_call () =
  let msg = build_request_message "autocomplete"
    {|
      "filename" : "test.php",
      "position" : {
        "line" : 9,
        "column" : 10
      }
  |} in
  test msg @@
  expect_api_message (
    Autocomplete {
      filename = "test.php";
      position = {line = 9; column = 10};
    }
  )

let tests = [
  "test_did_open_file", test_did_open_file;
  "test_autocomplete_call", test_autocomplete_call;
]

let () =
  Unit_test.run_all tests

(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_api_types
open Ide_message
open Ide_message_parser_test_utils
open Ide_rpc_protocol_parser_types

(* Test suite for Nuclide-rpc version of the API methods *)

let build_call_message = Printf.sprintf
  {|{
    "protocol" : "service_framework3_rpc",
    "id" : 4,
    "type" : "call",
    "method" : "%s" ,
    "args" : {%s}
  }|}

let test_method_not_found () =
  let msg = build_call_message "no_such_method" "" in
  test msg @@ (expect_error @@ Method_not_found "no_such_method")

let test_autocomplete_call () =
  let msg = build_call_message "getCompletions"
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

let test_coverage_levels_call () =
  let msg = build_call_message "coverageLevels"
    {|
      "filename" : "test.php"
    |}
  in
  test msg @@
  expect_api_message (Coverage_levels "test.php")

let test_did_open_file () =
  let msg = build_call_message "didOpenFile"
    {|
      "filename" : "test.php",
      "contents" : "<?hh"
    |} in
  test msg @@
  expect_api_message (
    Did_open_file {
      did_open_file_filename = "test.php";
      did_open_file_text =  "<?hh";
    }
  )

let test_did_close_file () =
  let msg = build_call_message "didCloseFile"
    {|
      "filename" : "test.php"
    |};
  in
  test msg @@
  expect_api_message (Did_close_file {did_close_file_filename = "test.php"})

let test_did_change_file () =
  let msg = build_call_message "didChangeFile"
    {|
      "filename" : "test.php",
      "changes" : [
        {
          "range" : {
            "start" : {
              "line" : 0,
              "column" : 0
            },
            "end" : {
              "line" : 0,
              "column" : 2
            }
          },
          "text" : "<?hh"
        },
        {
          "range" : {
            "start" : {
              "line" : 2,
              "column" : 2
            },
            "end" : {
              "line" : 2,
              "column" : 4
            }
          },
          "text" : "class"
        }
      ]
    |} in
  test msg @@
  expect_api_message (Did_change_file {
    did_change_file_filename = "test.php";
    changes = [
      {
        range = Some {
          st = {line = 0; column = 0};
          ed = {line = 0; column = 2}
        };
        text = "<?hh"
      };
      {
        range = Some {
          st = {line = 2; column = 2};
          ed = {line = 2; column = 4}
        };
        text = "class"
      };
    ]
  })


let test_did_change_file2 () =
  let msg = build_call_message "didChangeFile"
    {|
      "filename" : "test.php",
      "changes" : [
        {
          "text" : "<?hh"
        },
        {
          "range" : {
            "start" : {
              "line" : 2,
              "column" : 2
            },
            "end" : {
              "line" : 2,
              "column" : 4
            }
          },
          "text" : "class"
        }
      ]
    |} in
  test msg @@
  expect_api_message (Did_change_file {
    did_change_file_filename = "test.php";
    changes = [
      {
        range = None;
        text = "<?hh"
      };
      {
        range = Some {
          st = {line = 2; column = 2};
          ed = {line = 2; column = 4}
       };
       text = "class"
     };
   ]
 })

let test_subscribe () =
  let msg = build_call_message "notifyDiagnostics" "" in
  test msg @@
  expect_api_message Subscribe_diagnostics

let test_unsubscribe () =
  let msg = {|{
    "protocol" : "service_framework3_rpc",
    "id" : 4,
    "type" : "unsubscribe"
  }|} in
  test msg @@
  expect_api_message Ide_message.Unsubscribe_call

let test_disconnect () =
  let msg = build_call_message "disconnect" "" in
  test msg @@ expect_api_message Disconnect

let tests = [
  "test_method_not_found", test_method_not_found;
  "test_autocomplete_call", test_autocomplete_call;
  "test_coverage_levels_call", test_coverage_levels_call;
  "test_did_open_file", test_did_open_file;
  "test_did_close_file", test_did_close_file;
  "test_did_change_file", test_did_change_file;
  "test_did_change_file2", test_did_change_file2;
  "test_subscribe", test_subscribe;
  "test_unsubscribe", test_unsubscribe;
  "test_disconnect", test_disconnect;
]

let () =
  Unit_test.run_all tests

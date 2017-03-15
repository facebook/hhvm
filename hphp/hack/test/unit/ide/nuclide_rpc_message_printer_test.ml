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

(* Test suite for Nuclide-rpc version of the API responses  *)

let test_response = test_response Nuclide_rpc V0
let test_request = test_request Nuclide_rpc V0
let test_notification = test_notification Nuclide_rpc V0

let test_autocomplete_response () =
  let response = Autocomplete_response [{
    autocomplete_item_text = "aaa";
    autocomplete_item_type = "bbb";
    callable_details = Some {
      return_type = "ccc";
      Ide_message.callable_params = [{
          callable_param_name  = "ddd";
          callable_param_type = "eee";
      }]
    }
  }] in
  test_response response
  {|{
    "protocol": "service_framework3_rpc",
    "type": "response",
    "id": 4,
    "result": [
      {
        "name": "aaa",
        "type": "bbb",
        "pos": {"filename":"","line":0,"char_start":0,"char_end":-1},
        "expected_ty":false,
        "func_details": {
          "return_type": "ccc",
          "params": [
            {
              "name": "ddd",
              "type": "eee",
              "variadic": false
            }
          ],
          "min_arity": 0
        }
      }
    ]
  }|}

let test_infer_type_response () =
  test_response (Infer_type_response (Some "hello"))
  {|{
    "protocol": "service_framework3_rpc",
    "type": "response",
    "id": 4,
    "result": {
      "type": "hello",
      "pos": {"filename":"","line":0,"char_start":0,"char_end":-1}
    }
  }|}
  &&
  test_response (Infer_type_response None)
  {|{
    "protocol": "service_framework3_rpc",
    "type": "response",
    "id": 4,
    "result": {
      "type": null,
      "pos": {"filename":"","line":0,"char_start":0,"char_end":-1}
    }
  }|}

let test_coverage_levels_response () =
  test_response (Coverage_levels_response (
      Deprecated_text_span_coverage_levels_response [
      (Some Checked, "foo");
      (Some Partial, "bar");
      (Some Unchecked), "baz";
      (None, "qux");
    ]
  ))
  {|{
    "protocol": "service_framework3_rpc",
    "type": "response",
    "id": 4,
    "result": [
      {"color": "checked", "text": "foo"},
      {"color": "partial", "text": "bar"},
      {"color": "unchecked", "text": "baz"},
      {"color": "default", "text": "qux"}
    ]
  }|}

let test_diagnostics_notification () =
  let notification = Diagnostics_notification {
    subscription_id = 4;
    diagnostics_notification_filename = "foo.php";
    diagnostics = []
  } in
  test_notification notification
  {|{
    "protocol": "service_framework3_rpc",
    "type": "next",
    "id": 4,
    "value": {
      "filename": "foo.php",
      "errors": []
    }
  }|}

let tests = [
  "test_autocomplete_response", test_autocomplete_response;
  "test_diagnostics_notification", test_diagnostics_notification;
  "test_infer_type_response", test_infer_type_response;
  "test_coverage_levels_response", test_coverage_levels_response;
]

let () =
  Unit_test.run_all tests

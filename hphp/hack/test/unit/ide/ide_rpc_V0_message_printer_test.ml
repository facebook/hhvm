(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

 open Ide_message_parser_test_utils
 open Ide_rpc_protocol_parser_types

(* Test suite for V0 version of the API responses  *)

let test_response = test_response JSON_RPC2 V0

let test_error error expected =
  let response = Json_rpc_message_printer.response_to_json
    ~id:(Some 4)
    ~result:(`Error error)
  in
  assert_json_equal expected response;
  true

let test_method_not_found () =
  let error = Method_not_found "no_such_method" in
  test_error error
  {|{
    "jsonrpc": "2.0",
    "id": 4,
    "error": {
      "code": -32601,
      "message": "Method not found: no_such_method"
    }
  }|}

let tests = [
  "test_method_not_found", test_method_not_found;
]

let () =
  Unit_test.run_all tests

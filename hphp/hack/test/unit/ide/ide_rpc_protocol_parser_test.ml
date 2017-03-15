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

(* Test suite exercising RPC-protocol specific parts of the parser *)

let test message check_function =
  check_function (Ide_rpc_protocol_parser.parse ~version:V0 ~message);
  true

let expect_id_error expected got = expect_error_helper expected got.id

let expect_method_name expected got = expect_value
  (fun got -> got.method_name) expected got

let expect_params expected got = expect_value
  (fun got -> got.params) expected got

let test_invalid_json () = test
  "NOT A VALID JSON STRING"
  (expect_error @@
    Parse_error "expected '{[\"0123456789' or {t,f,n} at char[0]=N")

let test_non_object () = test
  "[]"
  (expect_error @@ Invalid_request "Value is not an object ")

let test_no_protocol () = test
  "{}"
  (expect_error @@ Invalid_request "Missing key: protocol")

let test_unkown_protocol () = test
  {|{"protocol" : "not_a_valid_protocol"}|}
  (expect_error @@ Invalid_request "Unknown protocol: not_a_valid_protocol")

let test_protocol_not_string () = test
  {|{"protocol" : 123}|}
  (expect_error
    @@ Invalid_request "Value expected to be String (at field [protocol])")

let test_call_id_not_int () = test
  {|{"protocol" : "service_framework3_rpc", "id" : "aaa"}|}
  (expect_id_error
    @@ Invalid_request "Value expected to be Number (at field [id])")

let test_call_id_not_int2 () = test
  {|{"protocol" : "service_framework3_rpc", "id" : 12.3}|}
  (expect_id_error
    @@ Invalid_request "field must be an integer or null, got: 12.3")

let test_no_type () = test
  {|{"protocol" : "service_framework3_rpc", "id" : 4}|}
  (expect_error
    @@ Invalid_request "Missing key: type")

let test_type_not_recognized () = test
  {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "aaaa"}|}
  (expect_error
    @@ Invalid_request "Message type not recognized: aaaa")

let test_no_method () = test
  {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "call"}|}
  (expect_error
    @@ Invalid_request "Missing key: method")

let test_args_not_object () = test
  {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "call",
    "method" : "open", "args" : 4}|}
  (expect_error
    @@ Invalid_request "Value expected to be Object (at field [args])")

let test_method_name () = test
  {|{"protocol" : "service_framework3_rpc", "id" : 4, "type" : "call",
    "method" : "method_name", "args" : {}}|}
  (expect_method_name @@ Method_name "method_name")

let test_unsubscribe_call () = test
  {|{"protocol":"service_framework3_rpc","id":4,"type":"unsubscribe"}|}
  (expect_method_name Unsubscribe_call)

let test_jsonrpc_method_name () = test
  {|{"jsonrpc":"2.0","method":"method_name"}|}
  (expect_method_name @@ Method_name "method_name")

let test_jsonrpc_params_name () = test
  {|{"jsonrpc":"2.0","method":"x","params":{"aaa": "bbb"}}|}
  (expect_params @@ Some Hh_json.(JSON_Object [
    "aaa", JSON_String "bbb"
  ]))

let tests = [
  "test_invalid_json", test_invalid_json;
  "test_non_object", test_non_object;
  "test_no_protocol", test_no_protocol;
  "test_unkown_protocol", test_unkown_protocol;
  "test_protocol_not_string", test_protocol_not_string;
  "test_call_id_not_int", test_call_id_not_int;
  "test_call_id_not_int2", test_call_id_not_int2;
  "test_no_type", test_no_type;
  "test_type_not_recognized", test_type_not_recognized;
  "test_no_method", test_no_method;
  "test_args_not_object", test_args_not_object;
  "test_method_name", test_method_name;
  "test_unsubscribe_call", test_unsubscribe_call;
  "test_jsonrpc_method_name", test_jsonrpc_method_name;
  "test_jsonrpc_params_name", test_jsonrpc_params_name;
]

let () =
  Unit_test.run_all tests

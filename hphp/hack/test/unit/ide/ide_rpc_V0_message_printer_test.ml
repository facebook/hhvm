(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_message
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

let test_init_response () =
  let response = Init_response {
    server_api_version = 4;
  } in
  test_response response
  {|{
    "jsonrpc": "2.0",
    "id": 4,
    "result": {
      "server_api_version": 4
    }
  }|}

let test_identify_symbol_response () =
  let occurrence =
    let open SymbolOccurrence in
    {
      name = "aaa";
      type_ = Class;
      pos = Pos.(to_absolute none)
    }
  in
  let definition =
    let open SymbolDefinition in
    Some {
      kind = Trait;
      name = "bbb";
      full_name = "ccc";
      SymbolDefinition.id = None;
      pos = Pos.(to_absolute none);
      span = Pos.(to_absolute none);
      modifiers = [];
      children = None;
      params = None;
      docblock  = None
    }
  in
  let response = Identify_symbol_response [
    {occurrence; definition}
  ] in
  test_response response
  {|{
    "jsonrpc": "2.0",
    "id": 4,
    "result": [
      {
        "name": "aaa",
        "kind": "type_id",
        "span": {
          "start": {
            "line": 0,
            "column": 0
          },
          "end": {
            "line": 0,
            "column": 0
          }
        },
        "definition": {
          "name": "bbb",
          "kind": "trait",
          "position": {
            "line": 0,
            "column": -1
          },
          "span": {
            "start": {
              "line": 0,
              "column": 0
            },
            "end": {
              "line": 0,
              "column": 0
            }
          },
          "modifiers": [

          ],
          "filename": ""
        }
      }
    ]
  }|}

let test_outline_response () =
  let open SymbolDefinition in
  let response = Outline_response [
    {
      kind = Function;
      name = "bbb";
      full_name = "ccc";
      SymbolDefinition.id = None;
      pos = Pos.(to_absolute none);
      span = Pos.(to_absolute none);
      modifiers = [];
      children = None;
      params = None;
      docblock  = None
    }
  ] in
  test_response response
  {|{
    "jsonrpc": "2.0",
    "id": 4,
    "result": [
      {
        "name": "bbb",
        "kind": "function",
        "position": {
          "line": 0,
          "column": -1
        },
        "span": {
          "start": {
            "line": 0,
            "column": 0
          },
          "end": {
            "line": 0,
            "column": 0
          }
        },
        "modifiers": [

        ]
      }
    ]
  }|}

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
    "jsonrpc": "2.0",
    "id": 4,
    "result": [
      {
        "name": "aaa",
        "type": "bbb",
        "callable_details": {
          "return_type": "ccc",
          "params": [
            {
              "name": "ddd",
              "type": "eee"
            }
          ]
        }
      }
    ]
  }|}

let test_infer_type_response () =
  let response = Infer_type_response (Some "foo") in
  test_response response
  {|{
    "jsonrpc": "2.0",
    "id": 4,
    "result": "foo"
  }|}
  &&
  let response = Infer_type_response None in
  test_response response
  {|{
    "jsonrpc": "2.0",
    "id": 4,
    "result": null
  }|}


let tests = [
  "test_method_not_found", test_method_not_found;
  "test_init_response", test_init_response;
  "test_identify_symbol_response", test_identify_symbol_response;
  "test_outline_response", test_outline_response;
  "test_autocomplete_response", test_autocomplete_response;
  "test_infer_type_response", test_infer_type_response;
]

let () =
  Unit_test.run_all tests

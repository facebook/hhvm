(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_json
open Ide_message
open Ide_rpc_protocol_parser

let build_message id fields =
  JSON_Object (
    ["jsonrpc", JSON_String "2.0"]
    @ Ide_parser_utils.opt_field id "id" int_
    @ fields
  )

let success_to_json id result =
  build_message id ["result", result]

let error_to_json ~id ~error =
  build_message id  ["error", JSON_Object [
    "code", int_ (error_t_to_code error);
    "message", JSON_String (error_t_to_string error);
  ]]

let request_to_json r params_as_json =
  build_message None [
    "method", JSON_String (request_method_name r);
    "params", params_as_json;
  ]

let to_json ~id ~message ~message_as_json = match message with
  | Response _ -> success_to_json id message_as_json
  | Request r -> request_to_json r message_as_json

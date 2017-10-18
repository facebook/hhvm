(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_rpc_protocol_parser_types

let fail s =
  print_endline s;
  assert false

let assert_equal expected got =
  if expected <> got then
    fail (Printf.sprintf "Expected %s, got %s" expected got)

let assert_errors_equal expected got =
  Ide_rpc_protocol_parser.(
    assert_equal (error_t_to_string expected) (error_t_to_string got)
  )

let expect_error_helper expected = function
  | Ok _ -> fail "Expected an error"
  | Error got -> assert_errors_equal expected got

let expect_error expected got = expect_error_helper expected got.result

let expect_value get_fun expected got : unit = match got.result with
  | Error e ->
    fail (Printf.sprintf "Unexpected error: %s"
      (Ide_rpc_protocol_parser.error_t_to_string e))
  | Ok got ->
    (* TODO: without struct printing, this error is not very useful *)
    if expected <> (get_fun got) then fail "Got unexpected result"

let assert_json_equal expected got =
  let open Hh_json in
  (* "Canonicalize" expected string, by parsing and unparsing it *)
  let expected = json_to_string (json_of_string expected) in
  let got = json_to_string got in
  assert_equal expected got

let expect_api_message expected got = expect_value
  (fun got -> got) expected got

let test message check_function =
  check_function (Ide_message_parser.parse ~version:V0 ~message);
  true

let test_message protocol version message expected =
  let message = Ide_message_printer.to_json
    ~id:(Some 4)
    ~protocol
    ~version
    ~message
  in
  assert_json_equal expected message;
  true

let test_response protocol version response expected =
  test_message protocol version (Ide_message.Response response) expected

let test_request protocol version request expected =
  test_message protocol version (Ide_message.Request request) expected

let test_notification protocol version notification expected =
  test_message protocol version
    Ide_message.(Request (Server_notification notification)) expected

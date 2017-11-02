(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * Ensure that errors in open file are never filtered, even if we are
 * throttling errors in other files to avoid overloading the editor.
 *)

open Integration_test_base_types
open Reordered_argument_collections

module Test = Integration_test_base

let foo_name = Printf.sprintf "foo%d.php"
let foo_contents = Printf.sprintf
"<?hh // strict

function foo%d (): string { return 4;}
"

let create_foo i = foo_name i, foo_contents i

let rec create_foos acc = function
  | 0 -> acc
  | i ->  create_foos ((create_foo i) :: acc) (i-1)

let get_diagnostics_map loop_output = match loop_output.push_message with
  | Some (ServerCommandTypes.DIAGNOSTIC (_, s)) -> s
  | _ ->
    Test.fail "Expected push diagnostics";
    assert false

let get_files_with_errors diagnostics_map =
  SSet.of_list (SMap.keys diagnostics_map)

let f123_diagnostics =
{|/foo123.php:
File "/foo123.php", line 3, characters 37-37:
Invalid return type (Typing[4110])
File "/foo123.php", line 3, characters 21-26:
This is a string
File "/foo123.php", line 3, characters 37-37:
It is incompatible with an int
|}

let () =
  (* Initialize a repo with errors in 200 files *)
  let env = Test.setup_server () in
  let disk_contests = create_foos [] 200 in
  let env = Test.setup_disk env disk_contests in

  (* After connecting, errors for 10 of them will be pushed to editor *)
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in

  let files_with_errors = loop_output |> get_diagnostics_map
    |> get_files_with_errors  in

  let num_errors = SSet.cardinal files_with_errors in
  if num_errors <> 10 then Test.fail
    (Printf.sprintf "Expected errors in 10 files, got: %d" num_errors);

  (* f123 will not be one of them *)
  let f123_name, f123_contents = create_foo 123 in
  if SSet.mem files_with_errors f123_name then Test.fail
    (Printf.sprintf "Expected errors for %s to be throttled" f123_name);

  (* But it should change after opening that file *)
  let env = Test.open_file env f123_name ~contents:f123_contents in
  let env = Test.wait env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output f123_diagnostics;
  ignore env

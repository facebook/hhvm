(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Integration_test_base_types
module Test = Integration_test_base

let foo_contents = "<?hh

function foo() {

}
"

let bar_contents = "<?hh

function test() {
  foo();
}"

let identify_foo_request =
  ServerCommandTypes.IDENTIFY_FUNCTION
    ("", ServerCommandTypes.FileContent bar_contents, 4, 4)

let check_identify_foo_response = function
  | Some [(_, Some def)] ->
    let string_pos = Pos.string def.SymbolDefinition.pos in
    let expected_pos = "File \"/foo.php\", line 3, characters 10-12:" in
    Test.assertEqual expected_pos string_pos
  | _ -> Test.fail "Expected to find exactly one definition"

let test () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [("foo.php", foo_contents)] in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          new_client = Some (RequestResponse identify_foo_request);
        })
  in
  check_identify_foo_response loop_output.new_client_response;

  let env = Test.connect_persistent_client env in
  let (_, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          persistent_client_request = Some (Request identify_foo_request);
        })
  in
  check_identify_foo_response loop_output.persistent_client_response

(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

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
  | [(_, def)] ->
    let string_pos = Pos.string def.SymbolDefinition.pos |> Test.relativize in
    let expected_pos = "File \"/foo.php\", line 3, characters 10-12:" in
    Test.assertEqual expected_pos string_pos
  | _ -> Test.fail "Expected to find exactly one definition"

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env = Test.Client.setup_disk env [("foo.php", foo_contents)] in
  let (env, response) =
    ClientIdeDaemon.Test.handle
      env
      (ClientIdeMessage.Definition
         (Test.doc "bar.php" bar_contents, Test.loc 4 4))
  in
  check_identify_foo_response response;

  ignore env;
  ()

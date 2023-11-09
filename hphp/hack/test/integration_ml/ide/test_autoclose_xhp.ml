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

let foo_contents =
  "<?hh
final class :xhp:foo extends :x:element {}
function test0(): void {
  <xhp:foo>

}
"

let check_identify_foo_response = function
  | Some close_tag -> Test.assertEqual close_tag "$0</xhp:foo>"
  | None -> Test.fail "Expected to insert the close tag $0</xhp:foo>"

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env = Test.Client.setup_disk env [("foo.php", foo_contents)] in
  let (env, response) =
    ClientIdeDaemon.Test.handle
      env
      (ClientIdeMessage.AutoClose
         (Test.doc "foo.php" foo_contents, Test.loc 4 12))
  in
  check_identify_foo_response response;

  ignore env;
  ()

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents = "<?hh
class C {
  public function foo() {

  }
}
"

let foo_contents_with_parse_error =
  "<?hh
class C {
  public function bar() {PARSE_ERROR

  }
}
"

let bar_name = "bar.php"

let bar_contents = "<?hh
function test() {
  (new C())->f
}
"

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  (* Two unsaved files. Autocomplete doesn't respect other unsaved files. *)
  let (env, _diagnostics) = Test.Client.edit_file env foo_name foo_contents in
  let (env, _diagnostics) = Test.Client.edit_file env bar_name bar_contents in
  let (env, response) =
    ClientIdeDaemon.Test.handle
      env
      ClientIdeMessage.(
        Completion
          ( Test.doc bar_name bar_contents,
            Test.loc 3 15,
            { is_manually_invoked = true } ))
  in
  Test.assert_ide_completions response [];

  (* save one file, so the other now will respect it *)
  let env = Test.Client.setup_disk env [(foo_name, foo_contents)] in
  let (env, response) =
    ClientIdeDaemon.Test.handle
      env
      ClientIdeMessage.(
        Completion
          ( Test.doc bar_name bar_contents,
            Test.loc 3 15,
            { is_manually_invoked = true } ))
  in
  Test.assert_ide_completions response ["foo"];

  (* even with parse errors, it is still respected *)
  let env =
    Test.Client.setup_disk env [(foo_name, foo_contents_with_parse_error)]
  in
  let (env, response) =
    ClientIdeDaemon.Test.handle
      env
      ClientIdeMessage.(
        Completion
          ( Test.doc bar_name bar_contents,
            Test.loc 3 15,
            { is_manually_invoked = true } ))
  in
  Test.assert_ide_completions response ["bar"];

  ignore env;
  ()

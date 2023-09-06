(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_takes_int_contents = "<?hh // strict

function foo(int $x) : void {}
"

let foo_takes_string_contents =
  "<?hh // strict

function foo(string $x) : void {}
"

let bar_name = "bar.php"

let bar_contents =
  "<?hh // strict

function test(mixed $x): void {
  foo($x);
}
"

let disk_diagnostics =
  {|
/bar.php:
File "/bar.php", line 4, characters 7-8:
Invalid argument (Typing[4110])
  File "/foo.php", line 3, characters 14-16:
  Expected `int`
  File "/bar.php", line 3, characters 15-19:
  But got `mixed`
|}

let ide_diagnostics =
  {|
/bar.php:
File "/bar.php", line 4, characters 7-8:
Invalid argument (Typing[4110])
  File "/foo.php", line 3, characters 14-19:
  Expected `string`
  File "/bar.php", line 3, characters 15-19:
  But got `mixed`
|}

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [(bar_name, bar_contents); (foo_name, foo_takes_int_contents)]
  in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_diagnostics_string diagnostics disk_diagnostics;

  (* an unsaved foo.php doesn't affect bar.php *)
  let (env, _diagnostics) =
    Test.Client.edit_file env foo_name foo_takes_string_contents
  in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_diagnostics_string diagnostics disk_diagnostics;

  (* saving it does affect bar.php *)
  let env =
    Test.Client.setup_disk env [(foo_name, foo_takes_string_contents)]
  in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_diagnostics_string diagnostics ide_diagnostics;

  (* save it back again *)
  let env = Test.Client.setup_disk env [(foo_name, foo_takes_int_contents)] in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_diagnostics_string diagnostics disk_diagnostics;

  ignore env;
  ()

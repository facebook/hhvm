(**
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

let foo_takes_string_contents = "<?hh // strict

function foo(string $x) : void {}
"

let bar_name = "bar.php"
let bar_contents = "<?hh // strict

function test(mixed $x): void {
  foo($x);
}
"

let disk_diagnostics = {|
File "/bar.php", line 4, characters 7-8:
Invalid argument (Typing[4110])
File "/foo.php", line 3, characters 14-16:
This is an int
File "/bar.php", line 3, characters 15-19:
It is incompatible with a mixed value
|}

let ide_diagnostics = {|
File "/bar.php", line 4, characters 7-8:
Invalid argument (Typing[4110])
File "/foo.php", line 3, characters 14-19:
This is a string
File "/bar.php", line 3, characters 15-19:
It is incompatible with a mixed value
|}

let () =

  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    (bar_name, bar_contents);
    (foo_name, foo_takes_int_contents);
  ] in

  let env, loop_output = Test.status env in
  Test.assert_status loop_output disk_diagnostics;

  let env = Test.connect_persistent_client env in
  let env = Test.open_file env foo_name ~contents:foo_takes_string_contents in

  let env, _ = Test.full_check env in
  let env, loop_output = Test.status env in
  Test.assert_status loop_output ide_diagnostics;

  let env, loop_output = Test.status ~ignore_ide:true env in
  Test.assert_status loop_output disk_diagnostics;


  let env, _ = Test.full_check env in
  let env, loop_output = Test.status ~ignore_ide:false env in
  Test.assert_status loop_output ide_diagnostics;
  ignore env

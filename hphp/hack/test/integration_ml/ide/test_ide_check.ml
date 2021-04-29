(**
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

let a_name = "a.php"

let a_contents1 =
  "<?hh // strict
class A {
  public function foo() : void {}
}
"

(* Parsing error *)
let a_contents2 = "<?hh // strict
{
"

(* Replace foo with bar, and also introduce local typing error *)
let a_contents3 =
  "<?hh // strict
class A {
  public function bar(int $x) : void {
    $x + 'a'; // bug
  }
}
"

let a_contents3_diagnostics =
  "/a.php:
File \"/a.php\", line 4, characters 10-12:
Typing error (Typing[4110])
  File \"/a.php\", line 4, characters 5-12:
  Expected `num` because this is used in an arithmetic operation
  File \"/a.php\", line 4, characters 10-12:
  But got `string`"

let b_name = "b.php"

let b_contents =
  "<?hh // strict
class B extends A {
  public function test() : void {
    $this->foo(); // this will be an error after we remove A::foo
  }
}
"

let final_global_diagnostics =
  "
/b.php:
File \"/b.php\", line 4, characters 12-14:
No instance method `foo` in `B` (Typing[4053])
  File \"/a.php\", line 3, characters 19-21:
  Did you mean `bar` instead?
  File \"/b.php\", line 2, characters 7-7:
  This is why I think it is an object of type B
  File \"/b.php\", line 2, characters 7-7:
  Declaration of `B` is here
"

let autocomplete_contents =
  "<?hh
function test(B $b) : void {
  $b->AUTO332 // this should return bar() after we do the edit
}
"

let test () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [(a_name, a_contents1); (b_name, b_contents)] in
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  (* Open a file and send two edits, both with errors, in quick succession. *)
  let env = Test.open_file env a_name in
  let (env, loop_output) = Test.edit_file env a_name a_contents2 in
  (* See that we don't compute diagnostics immediately, but batch them
   * with delay *)
  Test.assert_no_diagnostics loop_output;
  let (env, loop_output) = Test.edit_file env a_name a_contents3 in
  (* As above *)
  Test.assert_no_diagnostics loop_output;

  (* Wait for push diagnostics *)
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  (* Expect diagnostics only for the most recent version of the file *)
  Test.assert_diagnostics loop_output a_contents3_diagnostics;

  (* Check that edit is reflected in autocomplete results *)
  let (env, loop_output) = Test.autocomplete env autocomplete_contents in
  Test.assert_autocomplete loop_output ["bar"; "test"];

  (* Trigger global analysis *)
  let (_, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes =
            [
              (* The actual change doesn't matter - saving anything to disk just
               * happens to currently be a trigger for global recheck *)
              ("x.php", "");
            ];
        })
  in
  (* Global recheck produces full list of errors, including errors in b.php *)
  Test.assert_diagnostics loop_output final_global_diagnostics

(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open HoverService

module Test = Integration_test_base

let classname_call = "<?hh // strict
class ClassnameCall {
  static function foo(): int {
    return 0;
  }
}

function call_foo(): void {
  ClassnameCall::foo();
// ^9:4          ^9:18
}"

let classname_call_cases = [
  ("classname_call.php", 9, 4), [{ info = "ClassnameCall"; doc_block = None }];
  ("classname_call.php", 9, 18), [{ info = "foo(): int"; doc_block = None }];
]

let chained_calls = "<?hh // strict
class ChainedCalls {
  public function foo(): this {
    return $this;
  }
}

function test(): void {
  $myItem = new ChainedCalls();
  $myItem
    ->foo()
    ->foo()
    ->foo();
//     ^13:8
}"

let chained_calls_cases = [
  ("chained_calls.php", 13, 8), [{ info = "foo(): ChainedCalls"; doc_block = None }];
]

let multiple_potential_types = "<?hh // strict
class C1 { public function foo(): int { return 5; } }
class C2 { public function foo(): string { return 's'; } }
function test_multiple_type(C1 $c1, C2 $c2, bool $cond): arraykey {
  $x = $cond ? $c1 : $c2;
  return $x->foo();
//        ^6:11^6:16
}"

let multiple_potential_types_cases = [
  ("multiple_potential_types.php", 6, 11), [{ info = "(C1 | C2)"; doc_block = None }];
  ("multiple_potential_types.php", 6, 16), [
    { info = "((function(): string) | (function(): int))"; doc_block = None };
    { info = "((function(): string) | (function(): int))"; doc_block = None };
  ];
]

let classname_variable = "<?hh // strict
class ClassnameVariable {
  public static function foo(): void {}
}

function test_classname(): void {
  $cls = ClassnameVariable::class;
  $cls::foo();
// ^8:4  ^8:10
}"

let classname_variable_cases = [
  ("classname_variable.php", 8, 4), [{ info="classname<ClassnameVariable>"; doc_block=None }];

  (* TODO(wipi): make this return something useful. *)
  ("classname_variable.php", 8, 10), [{ info="_"; doc_block=None }];
]

let files = [
  "classname_call.php", classname_call;
  "chained_calls.php", chained_calls;
  "classname_variable.php", classname_variable;
]

let cases =
  classname_call_cases
  @ chained_calls_cases
  @ classname_variable_cases

let () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env files in

  Test.assert_no_errors env;

  List.iter cases ~f:begin fun ((file, line, col), expectedHover) ->
    let list_to_string hover_list =
      let string_list = hover_list |> List.map ~f:HoverService.string_of_result in
      let inner = match string_list |> List.reduce ~f:(fun a b -> a ^ "; " ^ b) with
        | None -> ""
        | Some s -> s
      in
      Printf.sprintf "%s:%d:%d: [%s]" file line col inner
    in
    let fn = ServerUtils.FileName ("/" ^ file) in
    let hover = ServerHover.go env (fn, line, col) in
    Test.assertEqual
      (list_to_string expectedHover)
      (list_to_string hover)
  end

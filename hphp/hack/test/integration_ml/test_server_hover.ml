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

let id = "<?hh // strict
function id(int $x): int {
  return $x;
  //     ^3:10
}
"

let id_cases = [
  ("id.php", 3, 10), [{ info = "int"; doc_block = None }];
]

let class_A = "<?hh // strict
class A {
  public function __construct(
    private int $id,
  ) {}
  public function getId(): int {
    return $this->id;
    //     ^7:12  ^7:19
  }
}
"

let class_A_cases = [
  ("A.php", 7, 12), [{ info = "<static>"; doc_block = None }];
  ("A.php", 7, 19), [{ info = "int"; doc_block = None }];
]

let pair = "<?hh // strict
class Pair<T> {
  private T $fst;
  private T $snd;

  public function __construct(T $fst, T $snd) {
    $this->fst = $fst;
    $this->snd = $snd;
  }
  public function getFst(): T {
    return $this->fst;
  }
  public function setFst(T $fst): void {
    $this->fst = $fst;
  }
  public function getSnd(): T {
    return $this->snd;
  }
  public function setSnd(T $snd): void {
    $this->snd = $snd;
  }
}
"

let test_pair = "<?hh // strict
class B extends A {}
class C extends A {}

function test_pair(Pair<A> $v): Pair<A> {
  $c = $v->getSnd();
//     ^6:8  ^6:15
  $v = new Pair(new B(1), new C(2));
// ^8:4         ^8:17
  $v->setFst($c);
//           ^10:14
  return test_pair($v);
//       ^12:10    ^12:20
}
"

let test_pair_cases = [
  ("test_pair.php", 6, 8), [{ info = "Pair<A>"; doc_block = None }];
  ("test_pair.php", 6, 15), [{ info = "getSnd(): A"; doc_block = None }];
  ("test_pair.php", 8, 4), [{ info = "Pair"; doc_block = None }];
  ("test_pair.php", 8, 17), [{ info = "B"; doc_block = None }];
  ("test_pair.php", 10, 14), [{ info = "A"; doc_block = None }];
  ("test_pair.php", 12, 10), [{ info = "test_pair(Pair<A> $v): Pair<A>"; doc_block = None }];
  ("test_pair.php", 12, 19), [{ info = "Pair<A>"; doc_block = None }];
  ("test_pair.php", 12, 20), [{ info = "Pair"; doc_block = None }];
]

let loop_assignment = "<?hh // strict
function use(mixed $item): void {}
function cond(): bool { return true; }
function loop_assignment(): void {
  $x = 1;
// ^5:4
  while (true) {
    use($x);
//      ^8:9
    if (cond())
      $x = 'foo';
//    ^11:7
  }
  use($x);
//    ^14:7
}
"

let loop_assignment_cases = [
  ("loop_assignment.php", 5, 4), [{ info = "int"; doc_block = None }];
  ("loop_assignment.php", 8, 9), [{ info = "(string | int)"; doc_block = None }];
  ("loop_assignment.php", 11, 7), [{ info = "string"; doc_block = None }];
  ("loop_assignment.php", 14, 7), [{ info = "(string | int)"; doc_block = None }];
]

let lambda1 = "<?hh // strict
function test_lambda1(): void {
  $s = 'foo';
  $f = $n ==> { return $n . $s . '\\n'; };
//^4:3                      ^4:29
  $x = $f(4);
//^6:3 ^6:8
  $y = $f('bar');
//^8:3    ^8:11
}
"

let lambda_cases = [
  ("lambda1.php", 4, 3), [{ info = "[fun]"; doc_block = None }];
  ("lambda1.php", 4, 29), [{ info = "string"; doc_block = None }];
  ("lambda1.php", 6, 3), [{ info = "string"; doc_block = None }];
  ("lambda1.php", 6, 8), [{ info = "string"; doc_block = None }];
  ("lambda1.php", 6, 11), [{ info = "int"; doc_block = None }];
  ("lambda1.php", 8, 3), [{ info = "string"; doc_block = None }];
  ("lambda1.php", 8, 8), [{ info = "string"; doc_block = None }];
  ("lambda1.php", 8, 11), [{ info = "string"; doc_block = None }];
]

let callback = "<?hh // strict
function test_callback((function(int): string) $cb): void {
  $cb;
//^3:3
  $cb(5);
//^5:3 ^5:8
  test_callback($cb);
//^7:3
}
"

let callback_cases = [
  ("callback.php", 3, 3), [{ info = "(function(int): string)"; doc_block = None }];
  ("callback.php", 5, 3), [{ info = "(function(int): string)"; doc_block = None }];
  ("callback.php", 5, 8), [{ info = "string"; doc_block = None }];
  ("callback.php", 7, 3), [
    { info = "test_callback(\n  (function(int): string) $cb\n): void"; doc_block = None }];
]

let invariant_violation = "<?hh // strict
class Exception {}
function invariant_violation(string $msg): noreturn {
  throw new Exception();
}
"

let nullthrows = "<?hh // strict
function nullthrows<T>(?T $x): T {
  invariant($x !== null, 'got null');
//          ^3:13
  return $x;
//       ^5:10
}
"

let nullthrows_cases = [
  ("nullthrows.php", 3, 13), [{ info = "?T"; doc_block = None }];
  ("nullthrows.php", 5, 10), [{ info = "T"; doc_block = None }];
]

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
  "id.php", id;
  "A.php", class_A;
  "Pair.php", pair;
  "test_pair.php", test_pair;
  "loop_assignment.php", loop_assignment;
  "lambda1.php", lambda1;
  "callback.php", callback;
  "invariant_violation.php", invariant_violation;
  "nullthrows.php", nullthrows;
  "classname_call.php", classname_call;
  "chained_calls.php", chained_calls;
  "multiple_potential_types.php", multiple_potential_types;
  "classname_variable.php", classname_variable;
]

let cases =
    id_cases
  @ class_A_cases
  @ test_pair_cases
  @ loop_assignment_cases
  @ lambda_cases
  @ callback_cases
  @ nullthrows_cases
  @ classname_call_cases
  @ chained_calls_cases
  @ multiple_potential_types_cases
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

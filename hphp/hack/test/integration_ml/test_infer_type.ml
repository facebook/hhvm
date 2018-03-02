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

module Test = Integration_test_base

let id = "<?hh // strict
function id(int $x): int {
  return $x;
  //     ^3:10
}
"

let id_cases = [
  ("id.php", 3, 10), ("int", "int");
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
  ("A.php", 7, 12), ("<static>", "<static>");
  ("A.php", 7, 19), ("int", "int");
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
  ("test_pair.php", 6, 8), ("Pair<A>", "Pair<A>");
  ("test_pair.php", 6, 15), ("(function(): A)", "A");
  ("test_pair.php", 8, 4), ("Pair", "Pair");
  ("test_pair.php", 8, 17), ("B", "B");
  ("test_pair.php", 10, 14), ("A", "A");
  ("test_pair.php", 12, 10), ("(function(Pair<A> $v): Pair<A>)", "Pair<A>");
  ("test_pair.php", 12, 19), ("Pair<A>", "Pair<A>");
  ("test_pair.php", 12, 20), ("Pair", "Pair");
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
  ("loop_assignment.php", 5, 4), ("int", "int");
  ("loop_assignment.php", 8, 9), ("(string | int)", "(string | int)");
  ("loop_assignment.php", 11, 7), ("string", "string");
  ("loop_assignment.php", 14, 7), ("(string | int)", "(string | int)");
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
  ("lambda1.php", 4, 3), ("[fun]", "[fun]");
  ("lambda1.php", 4, 29), ("string", "string");
  ("lambda1.php", 6, 3), ("string", "string");
  ("lambda1.php", 6, 8), ("[fun]", "string");
  ("lambda1.php", 6, 11), ("int", "int");
  ("lambda1.php", 8, 3), ("string", "string");
  ("lambda1.php", 8, 8), ("[fun]", "string");
  ("lambda1.php", 8, 11), ("string", "string");
]

let callback = "<?hh // strict
function test_callback((function(int): string) $cb): void {
  $cb;
//^3:3
  $cb(5);
//^5:3 ^5:8
}
"

let callback_cases = [
  ("callback.php", 3, 3), ("(function(int): string)", "(function(int): string)");
  ("callback.php", 5, 3), ("(function(int): string)", "string");
  ("callback.php", 5, 8), ("string", "string");
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
  ("nullthrows.php", 3, 13), ("?T", "?T");
  ("nullthrows.php", 5, 10), ("T", "T");
]

let curried = "<?hh // strict
function curried(): (function(int): (function(bool): string)) {
  return $i ==> $b ==> $i > 0 && $b ? 'true' : 'false';
}
function test_curried(bool $cond): void {
  $f = () ==> curried();
  $f()(5)(true);
//^7:3
}
"

let curried_cases = [
  ("curried.php", 6, 3), (
"(function(): (function(int): (function(bool): string)))",
        "(function(): (function(int): (function(bool): string)))"
  );
  ("curried.php", 7, 3), (
"(function(): (function(int): (function(bool): string)))",
        "string"
  );
]

let multiple_type = "<?hh // strict
class C1 { public function foo(): int { return 5; } }
class C2 { public function foo(): string { return 's'; } }
function test_multiple_type(C1 $c1, C2 $c2, bool $cond): arraykey {
  $x = $cond ? $c1 : $c2;
  return $x->foo();
//       ^6:10
}
"

let multiple_type_cases = [
  ("multiple_type.php", 6, 10), ("(C1 | C2)", "(C1 | C2)");
  ("multiple_type.php", 6, 14), ("((function(): string) | (function(): int))", "(int | string)");
]

let lambda_param = "<?hh // strict
function takes_func((function (int): int) $f): void {}
function lambda_param(): void {
  $f1 = $s ==> 3;
//      ^4:9
  takes_func($x ==> $x);
//           ^6:14
}
"

let lambda_param_cases = [
  ("lambda_param.php", 4, 9), ("_", "_");
  ("lambda_param.php", 6, 14), ("int", "int");
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
  "curried.php", curried;
  "multiple_type.php", multiple_type;
  "lambda_param.php", lambda_param;
]

let cases =
    id_cases
  @ class_A_cases
  @ test_pair_cases
  @ loop_assignment_cases
  @ lambda_cases
  @ callback_cases
  @ nullthrows_cases
  @ curried_cases
  @ multiple_type_cases
  @ lambda_param_cases

let () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env files in

  Test.assert_no_errors env;

  List.iter cases ~f:begin fun
    ((file, line, col), (expected_type, expected_returned_type))
  ->

    let compare_type expected type_at =
      let ty_str =
        match type_at with
        | Some (env, ty) -> Typing_print.full_strip_ns env ty
        | None ->
          Test.fail (Printf.sprintf "No type inferred at %s:%d:%d" file line col);
          failwith "unreachable"
      in
      let fmt = Printf.sprintf "%s:%d:%d %s" file line col in
      Test.assertEqual (fmt expected) (fmt ty_str)
    in

    let fn = ServerUtils.FileName ("/" ^ file) in

    let ServerEnv.{tcopt; files_info; _} = env in
    let _, tast = ServerIdeUtils.check_file_input tcopt files_info fn in

    let ty = ServerInferType.type_at_pos tast line col in
    compare_type expected_type ty;

    let ty = ServerInferType.returned_type_at_pos tast line col in
    compare_type expected_returned_type ty
  end

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
  ("id.php", 3, 10), "int";
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
  ("A.php", 7, 12), "<static>";
  ("A.php", 7, 19), "int";
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
  ("test_pair.php", 6, 8), "Pair<A>";
  ("test_pair.php", 6, 15), "A";
  ("test_pair.php", 8, 4), "Pair";
  ("test_pair.php", 8, 17), "B";
  ("test_pair.php", 10, 14), "A";
  ("test_pair.php", 12, 10), "Pair<A>";
  ("test_pair.php", 12, 19), "Pair<A>";
  ("test_pair.php", 12, 20), "Pair";
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
  ("loop_assignment.php", 5, 4), "int";
  ("loop_assignment.php", 8, 9), "(string | int)";
  ("loop_assignment.php", 11, 7), "string";
  ("loop_assignment.php", 14, 7), "(string | int)";
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
  ("lambda1.php", 4, 3), "[fun]";
  ("lambda1.php", 4, 29), "string";
  ("lambda1.php", 6, 3), "string";
  ("lambda1.php", 6, 8), "string";
  ("lambda1.php", 6, 11), "int";
  ("lambda1.php", 8, 3), "string";
  ("lambda1.php", 8, 8), "string";
  ("lambda1.php", 8, 11), "string";
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
  ("callback.php", 3, 3), "(function(int): string)";
  ("callback.php", 5, 3), "string";
  ("callback.php", 5, 8), "string";
]

let files = [
  "id.php", id;
  "A.php", class_A;
  "Pair.php", pair;
  "test_pair.php", test_pair;
  "loop_assignment.php", loop_assignment;
  "lambda1.php", lambda1;
  "callback.php", callback;
]

let cases =
    id_cases
  @ class_A_cases
  @ test_pair_cases
  @ loop_assignment_cases
  @ lambda_cases
  @ callback_cases

let () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env files in

  Test.assert_no_errors env;

  List.iter cases ~f:begin fun ((file, line, col), expected_type) ->
    let fn = ServerUtils.FileName ("/" ^ file) in
    let ty = ServerInferType.go env (fn, line, col) in
    let ty_name, _ty_json =
      match ty with
      | Some ty -> ty
      | None ->
        Test.fail (Printf.sprintf "No type inferred at %s:%d:%d" file line col);
        failwith "unreachable"
    in
    Test.assertEqual expected_type ty_name
  end

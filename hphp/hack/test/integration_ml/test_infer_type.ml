(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

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
  ("test_pair.php", 12, 20), "Pair";
]

let files = [
  "id.php", id;
  "A.php", class_A;
  "Pair.php", pair;
  "test_pair.php", test_pair;
]

let cases =
    id_cases
  @ class_A_cases
  @ test_pair_cases

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

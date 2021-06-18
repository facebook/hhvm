(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude
module Test = Integration_test_base

let id =
  "<?hh // strict
function id(int $x): int {
  return $x;
  //     ^3:10
}
"

let id_cases = [(("id.php", 3, 10), "int")]

let class_A =
  "<?hh // strict
class A {
  public function __construct(
    private int $id,
  ) {}
  public function getId(): int {
    return $this->id;
    //     ^7:12  ^7:19
  }
  public static function foo(): int { return 3; }
  public static int $foo = 3;
  const int foo = 5;
}
"

let class_A_cases = [(("A.php", 7, 12), "this"); (("A.php", 7, 19), "int")]

let mypair =
  "<?hh // strict
class MyPair<T> {
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

let test_mypair =
  "<?hh // strict
class B extends A {}
class C extends A {}

function test_mypair(MyPair<A> $v): MyPair<A> {
  $c = $v->getSnd();
//     ^6:8  ^6:15
  $v = new MyPair(new B(1), new C(2));
// ^8:4           ^8:19
  $v->setFst($c);
//           ^10:14
  return test_mypair($v);
//       ^12:10      ^12:22
}
"

let test_mypair_cases =
  [
    (("test_mypair.php", 6, 8), "MyPair<A>");
    (("test_mypair.php", 6, 15), "(function(): A)");
    (("test_mypair.php", 6, 19), "A");
    (("test_mypair.php", 8, 4), "MyPair<A>");
    (("test_mypair.php", 8, 19), "B");
    (("test_mypair.php", 10, 14), "A");
    (("test_mypair.php", 12, 10), "(function(MyPair<A> $v): MyPair<A>)");
    (("test_mypair.php", 12, 21), "MyPair<A>");
    (("test_mypair.php", 12, 22), "MyPair<A>");
  ]

let loop_assignment =
  "<?hh // strict
function douse(mixed $item): void {}
function cond(): bool { return true; }
function loop_assignment(): void {
  $x = 1;
// ^5:4
  while (cond()) {
    douse($x);
//        ^8:11
    if (cond())
      $x = 'foo';
//    ^11:7
  }
  douse($x);
//      ^14:9
}
"

let loop_assignment_cases =
  [
    (("loop_assignment.php", 5, 4), "int");
    (("loop_assignment.php", 8, 11), "(int | string)");
    (("loop_assignment.php", 11, 7), "string");
    (("loop_assignment.php", 14, 9), "(int | string)");
  ]

let callback =
  "<?hh // strict
function test_callback((function(int): string) $cb): void {
  $cb;
//^3:3
  $cb(5);
//^5:3 ^5:8
}
"

let callback_cases =
  [
    (("callback.php", 3, 3), "(function(int): string)");
    (("callback.php", 5, 3), "(function(int): string)");
    (("callback.php", 5, 8), "string");
  ]

let nullthrows =
  "<?hh // strict
function nullthrows<T>(?T $x): T {
  invariant($x !== null, 'got null');
//          ^3:13
  return $x;
//       ^5:10
}
"

let nullthrows_cases =
  [
    (("nullthrows.php", 3, 13), "?T");
    (("nullthrows.php", 5, 10), "(nonnull & T)");
  ]

let nullvec =
  "<?hh // strict
function foo() : vec<?int> {
  $x = vec[1, null];
  $x;
//^4:3
  $y = vec[1, 'hi', null];
  $y;
//^7:3
  $z = vec[null];
  $z;
//^10:3
}
"

let nullvec_cases =
  [
    (("nullvec.php", 4, 3), "vec<?int>");
    (("nullvec.php", 7, 3), "vec<?(int | string)>");
    (("nullvec.php", 10, 3), "vec<null>");
  ]

let curried =
  "<?hh // strict
function curried(): (function(int): (function(bool): string)) {
  return $i ==> $b ==> $i > 0 && $b ? 'true' : 'false';
}
function test_curried(bool $cond): void {
  $f = () ==> curried();
  $f()(5)(true);
//^7:3
}
"

let curried_cases =
  [
    ( ("curried.php", 6, 3),
      "(function(): (function(int): (function(bool): string)))" );
    ( ("curried.php", 7, 3),
      "(function(): (function(int): (function(bool): string)))" );
  ]

let multiple_type =
  "<?hh // strict
class C1 { public function foo(): int { return 5; } }
class C2 { public function foo(): string { return 's'; } }
function test_multiple_type(C1 $c1, C2 $c2, bool $cond): arraykey {
  $x = $cond ? $c1 : $c2;
  return $x->foo();
//       ^6:10
}
"

let multiple_type_cases =
  [
    (("multiple_type.php", 6, 10), "(C1 | C2)");
    (("multiple_type.php", 6, 14), "(function(): (int | string))");
  ]

let lambda_param =
  "<?hh // strict
function takes_func((function (int): num) $f): void {}
function lambda_param(): void {
  $f1 = $s ==> 3;
//      ^4:9
  takes_func($x ==> $x);
//           ^6:14
}
"

let lambda_param_cases =
  [
    (("lambda_param.php", 4, 9), "mixed");
    (("lambda_param.php", 6, 14), "int");
    (("lambda_param.php", 4, 12), "(function(mixed $s): int)");
    (("lambda_param.php", 6, 17), "(function(int $x): num)");
  ]

let class_id =
  "<?hh // strict
function class_id(): void {
  $x = new A(5);
//         ^3:12
  A::foo();
//^5:3
  A::$foo;
//^7:3
  A::foo;
//^9:3
  $cls = A::class;
  $cls;
// ^12:4
}
"

let class_id_cases =
  [
    (("class_id.php", 3, 12), "A");
    (("class_id.php", 5, 3), "A");
    (("class_id.php", 7, 3), "A");
    (("class_id.php", 9, 3), "A");
    (("class_id.php", 12, 4), "classname<A>");
  ]

let dynamic_view =
  "<?hh
function any() {
  return 'any';
}
function foo($x) : void {
//           ^5:14
  $y = $x + 5;
//^7:3
  $z = $x + $x;
//^9:3
  $w = any();
//^11:3
  $z = $x ^ $x;
//^13:3
  $z = $w . '';
//^15:3
  $z = $x | 5;
//^17:3

}
"

let dynamic_view_cases =
  [
    (("dynamic_view.php", 5, 14), "dynamic");
    (("dynamic_view.php", 7, 3), "(dynamic | int)");
    (("dynamic_view.php", 9, 3), "dynamic");
    (("dynamic_view.php", 11, 3), "dynamic");
    (("dynamic_view.php", 13, 3), "dynamic");
    (("dynamic_view.php", 15, 3), "string");
    (("dynamic_view.php", 17, 3), "int");
  ]

let files =
  [
    ("id.php", id);
    ("A.php", class_A);
    ("MyPair.php", mypair);
    ("test_mypair.php", test_mypair);
    ("loop_assignment.php", loop_assignment);
    ("callback.php", callback);
    ("nullthrows.php", nullthrows);
    ("nullvec.php", nullvec);
    ("curried.php", curried);
    ("multiple_type.php", multiple_type);
    ("lambda_param.php", lambda_param);
    ("class_id.php", class_id);
    ("dynamic_view.php", dynamic_view);
  ]

let cases =
  id_cases
  @ class_A_cases
  @ test_mypair_cases
  @ loop_assignment_cases
  @ callback_cases
  @ nullthrows_cases
  @ nullvec_cases
  @ curried_cases
  @ multiple_type_cases
  @ lambda_param_cases
  @ class_id_cases

let test () =
  let env =
    Test.setup_server
      ()
      ~hhi_files:(Hhi.get_raw_hhi_contents () |> Array.to_list)
  in
  let env = Test.setup_disk env files in
  let test_case ~dynamic ((file, line, col), expected_type) =
    let compare_type expected type_at =
      let ty_str =
        match type_at with
        | Some (env, ty) -> Tast_env.print_ty env ty
        | None ->
          Test.fail
            (Printf.sprintf "No type inferred at %s:%d:%d" file line col)
      in
      let fmt = Printf.sprintf "%s:%d:%d %s" file line col in
      Test.assertEqual (fmt expected) (fmt ty_str)
    in
    let ctx =
      Provider_utils.ctx_from_server_env env
      |> Provider_context.map_tcopt ~f:(fun tcopt ->
             { tcopt with GlobalOptions.tco_dynamic_view = dynamic })
    in
    let (ctx, entry) =
      Provider_context.add_entry_if_missing
        ~ctx
        ~path:(Relative_path.from_root ~suffix:file)
    in
    let { Tast_provider.Compute_tast.tast; _ } =
      Tast_provider.compute_tast_unquarantined ~ctx ~entry
    in
    let ty = ServerInferType.type_at_pos ctx tast line col in
    compare_type expected_type ty
  in
  List.iter cases ~f:(test_case ~dynamic:false);
  List.iter dynamic_view_cases ~f:(test_case ~dynamic:true)

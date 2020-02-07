(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_core
module Test = Integration_test_base

let f1 =
  ( "f1.php",
    {|<?hh
// trivial case
function f_f1(): int {
  return 1;
}

class C {
  static ?int $x;
}

function g_f1(): void {
  $x = 1;
}
|}
  )

let f2 =
  ( "f2.php",
    {|<?hh
function f_f2(): void {
  $a = getx_f2();
}
async function getx_f2(): Awaitable<int> {
  return 1;
}
|}
  )

let f3 =
  ( "f3.php",
    {|<?hh
function f_f3(): void {
  g_f3(new A());
}

class A {}

<<__Rx>>
function g_f3(<<__OwnedMutable>> A $a): void {}
|}
  )

let f4 =
  ( "f4.php",
    {|<?hh
function f_f4(): void {
  $a = vec[vec[1]];
  $a[0][1] = 1;
}
|} )

let f5 =
  ("f5.php", {|<?hh
function f_f5(): int {
  $a = () ==> 1;
  return $a();
}
|})

let f6 =
  ( "f6.php",
    {|<?hh
function f_f6(Vector<A_f6> $v): void {
  $v[0] = new A_f6();
  $v[] = new A_f6();
}

class A_f6 {}
|}
  )

let f7 =
  ( "f7.php",
    {|<?hh
function f(): void {
  $a = g();
}

class A {}
<<__Rx, __ReturnsVoidToRx>>
function g(): A {
  return new A();
}
|}
  )

let f8 =
  ( "f8.php",
    {|<?hh

interface I_f8 {
  <<__Rx>>
  public function f(): int;
}

class A_f8 {
  <<__Rx, __OnlyRxIfImpl(I_f8::class)>>
  public function f(): int {
    return 1;
  }
}
class B_f8 extends A_f8 {
  public function f(): int {
    nonrx();
    return 1;
  }
}
function nonrx(): void {
}
|}
  )

let f9 =
  ( "f9.php",
    {|<?hh
function f_f9(): void {
  A_f9::$x = 1;
}

class A_f9 {
  public static ?int $x;
}
|}
  )

let f10 =
  ( "f10.php",
    {|<?hh
function f(): void {
  $a = new A_f10(42);
  $a->x = 5;
}

class A_f10 {
  public function __construct(public int $x) {
  }
}
|}
  )

let f11 =
  ( "f11.php",
    {|<?hh
class A_f11 {
  public function __construct() {
    $a = () ==> f_f11($this);
  }
}

<<__Rx>>
function f_f11(<<__MaybeMutable>> $a): void {
}|}
  )

let files : (string * string) list =
  [f1; f2; f3; f4; f5; f6; f7; f8; f9; f10; f11]

let normalize s =
  String.split_on_char '\n' s |> List.map ~f:String.trim |> String.concat ""

let tests : ((string * int * int) * string) list =
  [
    ( (fst f1, 2, 1),
      {|
    {"position":{"file":"/f1.php","line":2,"character":1},
     "error":"Function/method not found"}
  |}
    );
    ( (fst f1, 3, 10),
      {|
    {"position":{"file":"/f1.php","line":3,"character":10},"result":true}
  |}
    );
    ( (fst f1, 11, 10),
      {|
    {"position":{"file":"/f1.php","line":11,"character":10},"result":true}
  |}
    );
    ( (fst f2, 2, 10),
      {|
    {"position":{"file":"/f2.php","line":2,"character":10},
    "result":false,"errors":[{"message":[
    {"descr":"This value has Awaitable type. Awaitable typed values in reactive code must be immediately await'ed.",
    "path":"/f2.php","line":3,"start":8,"end":16,"code":4248}]}]}
  |}
    );
    ( (fst f3, 2, 10),
      {|
    {"position":{"file":"/f3.php","line":2,"character":10},
    "result":false,"errors":[{"message":[{
      "descr":"Invalid argument","path":"/f3.php","line":3,"start":8,"end":14,"code":4260
     },{
      "descr":"This parameter is marked with <<__OwnedMutable>>","path":"/f3.php","line":9,"start":36,"end":37,"code":4260
     },{
      "descr":"But this expression is not owned mutable","path":"/f3.php","line":3,"start":8,"end":14,"code":4260
     }]}]}
  |}
    );
    ( (fst f4, 2, 10),
      {|
    {"position":{"file":"/f4.php","line":2,"character":10},"result":true}
  |}
    );
    ( (fst f5, 2, 10),
      {|
    {"position":{"file":"/f5.php","line":2,"character":10},"result":true}
  |}
    );
    ( (fst f6, 2, 10),
      {|
    {"position":{"file":"/f6.php","line":2,"character":10},
    "result":false,"errors":[{"message":[{
      "descr":"Cannot assign to element of Hack Collection object via [] in a reactive context. Instead, use the 'set' method.",
      "path":"/f6.php","line":3,"start":3,"end":20,"code":4201}]},{"message":[{
      "descr":"Cannot append to a Hack Collection object in a reactive context. Instead, use the 'add' method.",
      "path":"/f6.php","line":4,"start":3,"end":19,"code":4201}]}]}
  |}
    );
    ( (fst f7, 2, 10),
      {|
    {"position":{"file":"/f7.php","line":2,"character":10},
    "result":false,"errors":[{"message":[{
      "descr":"Cannot use result of function annotated with <<__ReturnsVoidToRx>> in reactive context",
      "path":"/f7.php","line":3,"start":8,"end":10,"code":4247},{
      "descr":"This is function declaration.",
      "path":"/f7.php","line":8,"start":10,"end":10,"code":4247}]}]}
  |}
    );
    ( (fst f8, 15, 19),
      {|
    {"position":{"file":"/f8.php","line":15,"character":19},"result":true}
  |}
    );
    ( (fst f9, 2, 10),
      {|
    {"position":{"file":"/f9.php","line":2,"character":10},
    "result":false,"errors":[{"message":[{
      "descr":"Static property cannot be used in a reactive context.",
      "path":"/f9.php","line":3,"start":3,"end":10,"code":4228}]}]}
  |}
    );
    ( (fst f10, 2, 10),
      {|
    {"position":{"file":"/f10.php","line":2,"character":10},
    "result":false,"errors":[{"message":[{
      "descr":"This object's property is being mutated(used as an lvalue)\nYou cannot set non-mutable object properties in reactive functions",
      "path":"/f10.php","line":4,"start":3,"end":7,"code":4202}]}]}
  |}
    );
    ( (fst f11, 3, 19),
      {|
    {"position":{"file":"/f11.php","line":3,"character":19},
    "result":false,"errors":[{"message":[{
      "descr":"Neither a Mutable nor MaybeMutable object may be captured by an anonymous function.",
      "path":"/f11.php","line":4,"start":23,"end":27,"code":4283}]}]}  |}
    );
  ]
  |> List.map ~f:(fun (p, s) -> (p, normalize s))

let test () =
  let env =
    Test.setup_server
      ()
      ~hhi_files:(Hhi.get_raw_hhi_contents () |> Array.to_list)
  in
  let env = Test.setup_disk env files in
  let h = ServerFunIsLocallableBatch.handlers in
  let do_test ((file, line, col), expected) =
    let ctx = Provider_utils.ctx_from_server_env env in
    let pos_list = [(Relative_path.from_root file, line, col)] in
    let result = ServerRxApiShared.helper h ctx [] pos_list in
    if result <> [expected] then
      let msg =
        "Unexpected test result\nExpected:\n"
        ^ expected
        ^ "\nBut got:\n"
        ^ String.concat "\n" result
      in
      Test.fail msg
  in
  List.iter tests ~f:do_test

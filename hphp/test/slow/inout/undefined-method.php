<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function f(inout int $x): void {
    $x = 42;
  }
}

function test(dynamic $c): void {
  $x = 1;
  $c->g(inout $x);
  var_dump($x);
}

<<__EntryPoint>>
function main(): void {
  test(new C());
}

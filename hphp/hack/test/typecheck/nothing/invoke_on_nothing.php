<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function makeNothing<T>():T {
  throw new Exception("a");
}

class C {
  public function foo(mixed $m):int { return 3; }
}

function testit():void {
  $x = makeNothing();
  $x->foo(function(string $x): Awaitable<vec<int>> {
    switch ($x) { case "A" : return vec[2]; } });
}

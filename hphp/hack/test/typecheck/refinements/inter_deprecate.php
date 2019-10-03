<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class B implements I {
  <<__Deprecated('blah', 25)>>
  public function foo(): string {
    return "a";
  }
}

interface I {
  public function foo():string;
}

function testit(I $i):void {
  invariant($i as B, "B");
  $s =  $i->foo();
}

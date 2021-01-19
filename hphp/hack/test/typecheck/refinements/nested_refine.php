<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C { }
function bar<Ti>(Ti $value): Ti {
  if ($value !== null) {
    if ($value is C) return new C();
  }
  return $value;
}

class D extends C {
  public function foo():void { }
}
<<__EntryPoint>>
function breakit():void {
 $y = bar(new D());
 $y->foo();
}

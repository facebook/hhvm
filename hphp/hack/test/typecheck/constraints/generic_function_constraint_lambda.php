<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function bar():int;
}

function foo<T as I>(T $x): void {
  $f = (T $y) ==> $y->bar();
  $g = $y ==> $x->bar();
}

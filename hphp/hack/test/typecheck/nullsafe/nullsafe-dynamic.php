<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function foo():int { return 3; }
}

function foo(dynamic $x, ?C $c, bool $b):void {
  if ($b) $x = $c;
  $i = $x?->foo();
}

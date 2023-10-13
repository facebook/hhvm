<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B {
  public function foo():void { }
}

function run<Tu>(Tu $x): (function ((function(Tu):void)): void) {
  return $x ==> { return; };
}
function bar(C $c): void {
  // Generate v
  // (v, (function(v): void)): void
  // C <: v -- solve here, as v is constant in the type of the expression
  // (But: function parameter is like a return type, because we're checking more
  // more code under assumptions involving v)
  run($c)($d ==> $d->foo());
}

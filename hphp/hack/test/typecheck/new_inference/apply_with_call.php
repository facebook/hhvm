<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B {
  public function foo():void { }
}

function run<Tu>(Tu $x, (function(Tu):void) $y): void {
}
function runflip<Tu>((function(Tu):void) $y, Tu $x): void {
}
function expectC(C $c):void { }

function bar(C $c): void {
  run($c, $d ==> expectC($d));
  runflip($d ==> expectC($d), $c);
}

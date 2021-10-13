<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function g():int { return 3; }
class C {
  public static (function():int) $bar = g<>;
  public function __construct(public (function():int) $foo) { }
  public function foo():string { return "a"; }
  public static function bar():string { return "b"; }
}

function expectInt(int $_):void { }
function expectString(string $_):void { }

<<__EntryPoint>>
function testit():void {
  $c = new C(() ==> 1);
  C::$bar = () ==> 2;
  $x = $c->foo();
  $y = ($c->foo)();
  $f = $c->foo;

  $sx = $c::bar();
  $sy = $c::$bar();
  $sz = ($c::$bar)();

  expectString($x);
  expectInt($y);
  expectString($sx);
  expectInt($sy);
  expectInt($sz);
}

<?hh

interface I {
  public function foo(int $x, optional bool $y, optional string $z):void;
}

interface J extends I {
  public function foo(int $x, bool $y, string $z):void;
}

class C implements I {
  public function foo(int $x, bool $y = false, string $z = 'a'):void { }
}
class D implements I {
  public function foo(int $x, bool $y, string $z = 'a'):void { }
}
class E implements I {
  public function foo(int $x):void { }
}
abstract class F implements I {
  // Illegal: can't specify a default *and* declare it optional
  abstract public function foo(int $x, optional bool $y = false):void;
  // Illegal: method is not abstract
  public function bar(optional bool $z):void { }
}

function testit(I $i):void {
  $i->foo(3);
  $i->foo(false);
  $i->foo(4,true,"A");
}

// All should be rejected
interface K {
  // Optional parameter precedes non-optional one
  public function bad1(int $x, optional bool $y, string $z):void;
  // Optional parameter precedes non-optional one
  public function bad2(int $x, optional bool $y, optional float $z, string $w):void;
  // Optional inout parameter
  public function bad3(int $x, optional inout bool $y):void;
  // Optional variadic parameter
  public function bad4(int $x, optional bool... $y):void;
  // Optional parameter with default
  public function bad5(int $x, bool $y = false, string $z):void;
}

// Illegal: can't specify optional on a top-level function parameter
function top(optional int $x):void { }

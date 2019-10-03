<?hh

abstract class Base {
  abstract public function foo(string $x, string $y);
}

class D1 extends Base {
  public function foo(string $x, string $y) { return $x; }
}
class D2 extends Base {
  public function foo(string $x, inout string $y) { return $x; }
}


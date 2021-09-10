<?hh // strict

abstract class X {
  abstract const type T as arraykey = arraykey;

  final public function __construct(private this::T $val) {}

  public function get(): this::T {
    return $this->val;
  }

  private function set(this::T $x): void {
    $this->val = $x;
  }

  public static function swap(X $x1, X $x2): void {
    $x1->set($x2->get());
  }
}

class Y extends X {
  const type T = int;
}

function test(Y $y, X $x): void {
  X::swap($y, $x);
}

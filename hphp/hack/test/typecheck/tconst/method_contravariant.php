<?hh

abstract class X {
  abstract const type T as arraykey = string;

  private function __construct(private this::T $val) {}

  public function get(): this::T {
    return $this->val;
  }

  final public static function create(this::T $x): this {
    return new static($x);
  }

  private function test(this::T $x): (this, X, X, Y) {
    return
      tuple(static::create($x), self::create(''), XConcrete::create(''), Y::create(0));
  }
}

final class XConcrete extends X {}

class Y extends X {
  const type T = int;
}

function test(Y $y): (Y, X, Y) {
  return tuple($y::create(0), XConcrete::create(''), Y::create(0));
}

function error(X $x): void {
  $x::create('');
}

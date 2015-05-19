<?hh // strict

class X {
  const type T as arraykey = string;

  final private function __construct(private this::T $val) {}

  public function get(): this::T {
    return $this->val;
  }

  final public static function create(this::T $x): this {
    return new static($x);
  }

  private function test(this::T $x): (this, X, X, Y) {
    return tuple(
      static::create($x),
      self::create(''),
      X::create(''),
      Y::create(0),
    );
  }
}

class Y extends X {
  const type T = int;
}

function test(Y $y): (Y, X, Y) {
  return tuple($y::create(0), X::create(''), Y::create(0));
}

function error(X $x): void {
  $x::create('');
}

<?hh // strict

class X {
  const type T as arraykey = string;

  final public function __construct(
    private this::T $val,
  ) {}

  public function get(): this::T {
    return $this->val;
  }

  public function set(
    this::T $x
  ): void {
    $this->val = $x;
  }
}

class Y extends X {
  const type T = int;
}

function test(
  Y $y,
  X $x,
): void {
  swap($y, $x);
}

function swap(X $x1, X $x2): void {
  $x1->set($x2->get());
}

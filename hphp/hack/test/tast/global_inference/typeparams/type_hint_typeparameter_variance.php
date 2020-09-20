<?hh //partial

class C<+Tu, -Tv> {
  public function __construct(Tu $x, Tv $y) {}
}

function f(C $x): void {}

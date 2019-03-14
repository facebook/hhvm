<?hh // partial

class A {
  <<__Rx>>
  public function __construct(public int $x) {}

  <<__Rx>>
  public function f1(): void {
    $b = <<__NonRx>>() ==> {
      // OK - non-reactive lambda can capture anything
      g($this);
    };
  }

  <<__Rx>>
  public function f2(): void {
    $b = <<__NonRx>>() ==> {
      // OK - non-reactive lambda can capture anything
      $this->x = 42;
    };
  }
}

<<__Rx>>
function g(<<__MaybeMutable>> A $a): void {
}

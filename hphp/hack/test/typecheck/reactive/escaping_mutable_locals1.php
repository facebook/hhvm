<?hh

class A {
  <<__Rx>>
  public function __construct(public int $x) {}

  <<__Rx>>
  public function f1(): void {
    $b = () ==> {
      // OK - non-reactive lambda can capture anything
      g($this);
    };
  }

  <<__Rx>>
  public function f2(): void {
    $b = () ==> {
      // OK - non-reactive lambda can capture anything
      $this->x = 42;
    };
  }
}

<<__Rx>>
function g(<<__MaybeMutable>> A $a): void {
}

<<__Rx>>
function f1(): void {
  $a = \HH\Rx\mutable(new A(1));
  $b = () ==> {
    // OK - non-reactive lambda can capture anything
    $a->x = 42;
  };
}

<<__Rx>>
function f2(<<__Mutable>> A $a): void {
  $b = () ==> {
    // OK - non-reactive lambda can capture anything
    $a->x = 42;
  };
}

<<__Rx>>
function f3(<<__MaybeMutable>> A $a): void {
  $b = () ==> {
    // OK - non-reactive lambda can capture anything
    g($a);
  };
}

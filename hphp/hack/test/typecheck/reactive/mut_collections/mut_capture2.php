<?hh // partial

class A {
  <<__Rx>>
  public function __construct(public int $x) {}
}

<<__Rx>>
function f1(): void {
  $a = \HH\Rx\mutable(new A(1));
  $b = <<__NonRx>>() ==> {
    // Error
    $a->x = 42;
  };
}

<<__Rx>>
function f2(<<__Mutable>> A $a): void {
  $b = <<__NonRx>>() ==> {
    // Error
    $a->x = 42;
  };
}

<<__Rx>>
function f3(<<__MaybeMutable>> A $a): void {
  $b = <<__NonRx>>() ==> {
    // Error
    g($a);
  };
}

function g($_) {}

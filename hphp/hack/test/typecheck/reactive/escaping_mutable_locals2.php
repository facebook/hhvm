<?hh // partial

class A {
  <<__Rx>>
  public function __construct(public int $x) {}
}

<<__Rx>>
function f(<<__Mutable>> A $a): void {
  $b = <<__Rx>>() ==> {
    // ERROR - reactive lambga capturing mutable value
    $a->x = 42;
  };
}

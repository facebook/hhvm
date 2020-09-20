<?hh // partial

class A {
  <<__Rx>>
  public function __construct(public int $x) {}
}

<<__Rx>>
function g(<<__MaybeMutable>> A $a): void {
}

<<__Rx>>
function f(<<__MaybeMutable>> A $a): void {
  $b = <<__Rx>>() ==> {
    // ERROR - reactive lambga capturing mutable value
    g($a);
  };
}

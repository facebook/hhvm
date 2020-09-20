<?hh // partial

class A {
  <<__Rx>>
  public function __construct(public int $x) {}
  <<__Rx, __Mutable>>
  public function f(): void {
    $b = <<__Rx>>() ==> {
      // ERROR - reactive lambga capturing mutable value
      g($this);
    };
  }
}

<<__Rx>>
function g(<<__Mutable>> A $a): void {
}

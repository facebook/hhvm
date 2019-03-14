<?hh // partial

// OK
<<__Rx>>
function f(<<__OwnedMutable>> A $a) {
  // OK
  $x = <<__Rx>>(<<__OwnedMutable>> A $c) ==> {};
  // OK
  $z = <<__Rx>>function(<<__OwnedMutable>> A $c): void use($x) {
  };
}
class A {
  // OK
  <<__Rx>>
  public function f(<<__OwnedMutable>> A $a) {
  }
}

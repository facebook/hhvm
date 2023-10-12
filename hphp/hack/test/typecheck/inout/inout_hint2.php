<?hh // strict

type Tfun = (function(inout string): mixed);

class C1 {
  public function foo(Tfun $f): void {
    $x = 'C1';
    $f(inout $x);
  }
}

class C2 extends C1 {
  public function foo((function(inout string): mixed) $f): void {
    $x = 'C2';
    $f(inout $x);
  }
}

function foo(): (function(inout int): int) {
  return (inout $x) ==> 42;
}

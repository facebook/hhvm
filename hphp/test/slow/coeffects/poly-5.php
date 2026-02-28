<?hh

function rx(...$x)[rx] :mixed{ echo "in rx\n"; }

abstract class A {
  abstract const ctx C;
  public function f(...$x)[this::C] :mixed{
    rx(1,2,3);
  }
}

class B extends A { const ctx C = []; }

<<__EntryPoint>>
function main() :mixed{
  (new B)->f(1,2,3);
}

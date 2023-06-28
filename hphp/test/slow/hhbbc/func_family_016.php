<?hh

interface I {}

abstract class C1 implements I {
  public function foo() :mixed{ return 1; }
}
class C2 extends C1 {}
class C3 extends C1 {}

class C4 implements I {
  public function foo() :mixed{ return 2; }
}

<<__EntryPoint>>
function main(): void {
  echo "Done\n";
}

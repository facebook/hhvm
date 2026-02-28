<?hh

interface I {}

trait T1 implements I {
  private function foo() :mixed{ return 1; }
}
trait T2 implements I {
  private function foo() :mixed{ return 2; }
}

abstract class C1 {
  use T1;
}
abstract class C2 {
  use T2;
}
class C3 implements I {
  use T1;
}
class C4 implements I {
  use T2;
}

<<__EntryPoint>>
function main(): void {
  echo "Done\n";
}

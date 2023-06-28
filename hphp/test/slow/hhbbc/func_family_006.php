<?hh

abstract class Base {
  abstract public function foo(string $x, string $y):mixed;
}

class D1 extends Base {
  public function foo(string $x, string $y) :mixed{ return $x; }
}
class D2 extends Base {
  public function foo(string $x, inout string $y) :mixed{ return $x; }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}

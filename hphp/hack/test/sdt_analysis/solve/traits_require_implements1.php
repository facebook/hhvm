<?hh

interface I {
  public function foo(vec<int> $v): void;
}

trait Tr {
  require implements I;
  public function foo(vec<int> $_): void {}
}

class C implements I {
  use Tr;
}

function foo(I $i, dynamic $dyn): void {
  $i->foo($dyn);
}

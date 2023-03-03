<?hh

interface I {
  public function foo(vec<int> $_): void;
}

class C implements I {
  public function foo(vec<int> $_): void {}
}

function main(I $i, dynamic $d): void {
  $i->foo($d);
}

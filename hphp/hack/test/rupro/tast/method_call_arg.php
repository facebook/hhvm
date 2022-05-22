<?hh

interface I {
  public function foo(int $x, string $y): void;
}

function bar(I $i, int $x, string $y): void {
  $i->foo($x, $y);
}

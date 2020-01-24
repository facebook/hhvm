<?hh

final class Foo {
  private function bar(int $x): void {}
}

function baz(): void {
  $x = new Foo();
  $y = $x->bar<>;
  $y(4);
}

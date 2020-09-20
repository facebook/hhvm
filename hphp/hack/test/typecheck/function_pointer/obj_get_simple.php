<?hh

final class Foo {
  public function bar(int $x): void {

  }
}

function baz(): void {
  $x = new Foo();
  $y = $x->bar<>;
  $y(4);
}

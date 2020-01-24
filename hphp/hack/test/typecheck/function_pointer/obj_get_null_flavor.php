<?hh

final class Foo {
  public function bar(int $x): void {

  }
}

function baz(?Foo $x): void {
  $y = $x?->bar<>;
  $y(4);
}

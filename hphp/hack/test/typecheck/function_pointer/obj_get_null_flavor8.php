<?hh

final class Foo {
  public ?Foo $foo = null;

  public function bar(int $x): void {

  }
}

function baz(?Foo $x): void {
  $y = $x?->foo?->bar<>;
  if (!($y is null)) {
    $y(4);
  }
}

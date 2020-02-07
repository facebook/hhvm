<?hh

final class Foo {
  public function bar(int $x): void {

  }

  public function get_foo(): Foo {
    return new Foo();
  }
}

function baz(?Foo $x): void {
  $y = $x?->get_foo()?->bar<>;
  if (!($y is null)) {
    $y(4);
  }
}

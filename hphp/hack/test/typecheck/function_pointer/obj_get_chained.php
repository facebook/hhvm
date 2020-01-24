<?hh

final class Foo {
  public function bar(): Foo {
    return $this;
  }

  public function baz(int $x): void {}
}

function qux(): void {
  $x = new Foo();
  $y = $x->bar()->bar()->bar()->baz<>;
  $y(4);
}

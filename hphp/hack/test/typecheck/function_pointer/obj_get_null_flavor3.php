<?hh

final class Foo {
  public function not_bar(int $x): void {

  }
}

function baz(?Foo $x): void {
  $y = $x?->bar<>;
  if (!($y is null)) {
    $y(4);
  }
}

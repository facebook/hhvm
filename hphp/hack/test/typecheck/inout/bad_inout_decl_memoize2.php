<?hh

final class C {
  <<__Memoize>>
  public function bar(mixed $x, inout mixed $y, mixed $z): void {}
}

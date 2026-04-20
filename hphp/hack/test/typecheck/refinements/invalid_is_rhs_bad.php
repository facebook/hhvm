<?hh

enum MyEnum: int as int {
  ZERO = 0;
}

final class MyClass {}

function foo(mixed $x, dict<string, mixed> $d): void {
  0 is MyEnum::ZERO;
  $x is MyEnum::ZERO;
  $x is MyClass::class;
  if ($d['x'] is MyEnum::ZERO) {
  }
}

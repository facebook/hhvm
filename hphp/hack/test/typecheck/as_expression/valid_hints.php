<?hh

final class C {}

enum MyEnum: int {
  ONE = 1;
  TWO = 2;
}

function foo(mixed $x): void {
  $y = $x;
  if ($y as C) {
    expect_C($y);
  }
  $y = $x;
  if ($y as MyEnum) {
    expect_MyEnum($y);
  }
  $y = $x;
  if ($y as mixed !== null) {
    expect_mixed($y);
  }
  $y = $x;
  if ($y as ?string !== null) {
    expect_nstring($y);
  }
  $y = $x;
  if ($y as int) {
    expect_int($y);
  }
  $y = $x;
  if ($y as shape('foo' => int, ...)) {
    expect_shape($y);
  }
  $y = $x;
  if ($y as (int, ?string)) {
    expect_tuple($y);
  }
}

function expect_C(C $x): void {}
function expect_MyEnum(MyEnum $x): void {}
function expect_mixed(mixed $x): void {}
function expect_nstring(?string $x): void {}
function expect_int(int $x): void {}
function expect_shape(shape('foo' => int, ...) $x): void {}
function expect_tuple((int, ?string) $x): void {}

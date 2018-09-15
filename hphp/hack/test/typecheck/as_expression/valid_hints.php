<?hh // strict

final class C {}

enum MyEnum: int {
  ONE = 1;
  TWO = 2;
}

function foo(mixed $x): void {
  if ($x as C) {
    expect_C($x);
  } else if ($x as MyEnum) {
    expect_MyEnum($x);
  } else if ($x as mixed !== null) {
    expect_mixed($x);
  } else if ($x as ?string !== null) {
    expect_nstring($x);
  } else if ($x as int) {
    expect_int($x);
  } else if ($x as shape('foo' => int, ...)) {
    expect_shape($x);
  } else if ($x as (int, ?string)) {
    expect_tuple($x);
  }
}

function expect_C(C $x): void {}
function expect_MyEnum(MyEnum $x): void {}
function expect_mixed(mixed $x): void {}
function expect_nstring(?string $x): void {}
function expect_int(int $x): void {}
function expect_shape(shape('foo' => int, ...) $x): void {}
function expect_tuple((int, ?string) $x): void {}

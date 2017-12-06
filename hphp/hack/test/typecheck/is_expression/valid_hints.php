<?hh // strict

final class C {}

enum MyEnum: int {
  ONE = 1;
  TWO = 2;
}

function foo(mixed $x): void {
  if ($x is C) {
    expect_C($x);
  } else if ($x is MyEnum) {
    expect_MyEnum($x);
  } else if ($x is mixed) {
    expect_mixed($x);
  } else if ($x is ?string) {
    expect_nstring($x);
  } else if ($x is int) {
    expect_int($x);
  } else if ($x is shape('foo' => int, ...)) {
    expect_shape($x);
  } else if ($x is (int, ?string)) {
    expect_tuple($x);
  } else if ($x is classname<C>) {
    expect_classname($x);
  }
}

function expect_C(C $x): void {}
function expect_MyEnum(MyEnum $x): void {}
function expect_mixed(mixed $x): void {}
function expect_nstring(?string $x): void {}
function expect_int(int $x): void {}
function expect_shape(shape('foo' => int, ...) $x): void {}
function expect_tuple((int, ?string) $x): void {}
function expect_classname(classname<C> $x): void {}

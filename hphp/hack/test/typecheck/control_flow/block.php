<?hh // partial

class A {}
class B {}
class C {}

function expect_A(A $x): void {}
function expect_B(B $x): void {}
function expect_C(C $x): void {}

function test(): void {
  $x = new A();
  expect_A($x);
  {
    expect_A($x);
    $x = new B();
    expect_B($x);
  }
  expect_B($x);
  $x = new C();
  expect_C($x);
}

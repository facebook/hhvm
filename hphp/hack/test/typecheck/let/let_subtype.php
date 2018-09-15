<?hh // experimental

interface I {}
class A implements I {}

function expect_A(A $a): void {}
function expect_I(I $i): void {}

function foo(): void {
  let a : A = new A();
  expect_A(a);
  expect_I(a);
  let i : I = new A();
  expect_I(i);
  expect_A(i); // type error
}

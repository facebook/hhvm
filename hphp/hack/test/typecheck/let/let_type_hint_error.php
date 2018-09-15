<?hh // experimental

function expect_int(int $x): void {}

function foo(): void {
  let x : int = "Forty-two"; // type error
  expect_int(x); // *NO* type error
}

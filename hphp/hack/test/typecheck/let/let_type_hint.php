<?hh // experimental

function expect_int(int $x): void {}

function foo(): void {
  let x : int = 42;
  expect_int(x);
}

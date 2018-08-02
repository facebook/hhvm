<?hh // experimental

function expect_string(string $x): void {}

function foo(): void {
  let x : int = 42;
  expect_string(x); // type error
}

<?hh // experimental

function expect_int(int $x): void {}
function expect_string(string $x): void {}

function foo(): void {
  let x : int = 42;
  expect_int(x);
  let x : string = "Forty-two";
  expect_string(x);
}

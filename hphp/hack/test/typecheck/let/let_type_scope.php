<?hh // experimental

function expect_int(int $x): void {}
function expect_string(string $x): void {}

function foo(bool $b): void {
  let x : int = 42;
  if ($b) {
    expect_int(x);
    let x : string = "Forty-two";
    expect_string(x);
  }
  expect_int(x);
}

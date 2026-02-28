<?hh

function takes_int_makes_string(int $x): string {
  return "";
}
function expect_string(string $x): void {}

function test(int $x): void {
  if (($x = takes_int_makes_string($x)) === "") {
    expect_string($x);
  }
  expect_string($x);
}

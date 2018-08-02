<?hh // experimental

function expect_string(string $s): void {}

function foo(): void {
  let x = 10;
  let f = ($str) ==> {
    expect_string($str);
  };
  f(x);
}

<?hh // experimental

function expect_string(string $s): void {}

function foo(): void {
  let x = 10;
  let f = () ==> {
    expect_string(x);
  };
  f();
}

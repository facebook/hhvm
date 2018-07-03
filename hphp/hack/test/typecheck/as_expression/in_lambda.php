<?hh // strict

function f(mixed $x): void {
  $f = () ==> {
    expect_int($x as int);
  };
}

function expect_int(int $x): void {}

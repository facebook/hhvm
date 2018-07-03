<?hh // strict

function f(mixed $x): void {
  $f = () ==> {
    if ($x is int) {
      expect_int($x);
    }
  };
}

function expect_int(int $x): void {}

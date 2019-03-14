<?hh // partial

function f(mixed $x): void {
  if ($x is keyset) {
    expect_keyset1($x); // ok
    expect_keyset2($x); // error
  }
}

function g(mixed $x): void {
  if ($x is keyset<_>) {
    expect_keyset1($x); // ok
    expect_keyset2($x); // error
  }
}

function h(Container<string> $x): void {
  if ($x is keyset<_>) {
    expect_keyset1($x); // ok
    expect_keyset2($x); // ok
  }
}

function expect_keyset1(keyset<arraykey> $keyset): void {}
function expect_keyset2(keyset<string> $keyset): void {}

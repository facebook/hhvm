<?hh // partial

function f(mixed $x): void {
  if ($x is dict) {
    expect_dict1($x); // ok
    expect_dict2($x); // error
  }
}

function g(mixed $x): void {
  if ($x is dict<_, _>) {
    expect_dict1($x); // ok
    expect_dict2($x); // error
  }
}

function h(KeyedContainer<int, string> $x): void {
  if ($x is dict<_, _>) {
    expect_dict1($x); // ok
    expect_dict2($x); // ok
  }
}

function expect_dict1(dict<arraykey, mixed> $dict): void {}
function expect_dict2(dict<int, string> $dict): void {}

<?hh

function f((int, int) $x): void {
  expect_tuple($x as ?(int, arraykey));
  expect_tuple(tuple(0, 1) as ?(int, arraykey));
}

function g((int, int) $x): void {
  $x as ?(int, arraykey);
  expect_tuple($x);
}

function expect_tuple((int, int) $x): void {}

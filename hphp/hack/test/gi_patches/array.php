<?hh // partial

function f(array $v) {
  $v[3] = "foo";
  return $v[34897];
}

function g(array $v) {
  expect<string>($v[0]);
}

function h1(): array {
  return darray[0 => 'billie'];
}

function h2(): array {
  return varray['billie'];
}

function i(): array {
  $x = darray[];
  $x[3] = 'bob';
  return $x;
}

function expect<T>(T $_): void {}

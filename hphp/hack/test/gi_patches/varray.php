<?hh // partial

function f(varray $v) {
  $v[3] = "foo";
  return $v[34897];
}

function g(varray $v) {
  expect<string>($v[0]);
}

function h(): varray {
  return varray['billie', 'willie'];
}

function i(): varray {
  $x = varray[];
  $x[] = 'bob';
  return $x;
}

function expect<T>(T $_): void {}

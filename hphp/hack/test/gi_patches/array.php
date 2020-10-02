<?hh // partial

function f(varray_or_darray $v) {
  $v[3] = "foo";
  return $v[34897];
}

function g(varray_or_darray $v) {
  expect<string>($v[0]);
}

function h1(): varray_or_darray {
  return darray[0 => 'billie'];
}

function h2(): varray_or_darray {
  return varray['billie'];
}

function i(): varray_or_darray {
  $x = darray[];
  $x[3] = 'bob';
  return $x;
}

function expect<T>(T $_): void {}

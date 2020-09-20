<?hh // partial

function f(darray $v) {
  $v[3] = "foo";
  foreach ($v as $k => $v) {
    return $k;
  }
  return null;
}

function ff() {
  expect<?int>(f(darray[2 => "fs"]));
}

function g(darray $v) {
  expect<string>($v[0]);
  return $v;
}

function gg() {
  g(darray[0 => "sdf"]);
}

function h(): darray {
  return darray[0 => 'billie', 3 => 'willie'];
}

function i(): darray {
  $x = darray[];
  $x[3] = 'bob';
  return $x;
}

function expect<T>(T $_): void {}

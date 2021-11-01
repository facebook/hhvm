<?hh

/* HH_FIXME[4030] */
/* HH_FIXME[2071] */
function f(darray $v) {
  $v[3] = "foo";
  foreach ($v as $k => $v) {
    return $k;
  }
  return null;
}

/* HH_FIXME[4030] */
function ff() {
  expect<?int>(f(darray[2 => "fs"]));
}

/* HH_FIXME[4030] */
/* HH_FIXME[2071] */
function g(darray $v) {
  expect<string>($v[0]);
  return $v;
}

/* HH_FIXME[4030] */
function gg() {
  g(darray[0 => "sdf"]);
}

/* HH_FIXME[2071] */
function h(): darray {
  return darray[0 => 'billie', 3 => 'willie'];
}

/* HH_FIXME[2071] */
function i(): darray {
  $x = darray[];
  $x[3] = 'bob';
  return $x;
}

function expect<T>(T $_): void {}

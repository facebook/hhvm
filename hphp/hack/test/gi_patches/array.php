<?hh

/* HH_FIXME[4030] */
/* HH_FIXME[2071] */
function f(varray_or_darray $v) {
  $v[3] = "foo";
  return $v[34897];
}

/* HH_FIXME[4030] */
/* HH_FIXME[2071] */
function g(varray_or_darray $v) {
  expect<string>($v[0]);
}

/* HH_FIXME[2071] */
function h1(): varray_or_darray {
  return darray[0 => 'billie'];
}

/* HH_FIXME[2071] */
function h2(): varray_or_darray {
  return varray['billie'];
}

/* HH_FIXME[2071] */
function i(): varray_or_darray {
  $x = darray[];
  $x[3] = 'bob';
  return $x;
}

function expect<T>(T $_): void {}

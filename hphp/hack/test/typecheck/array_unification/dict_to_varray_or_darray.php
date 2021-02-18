<?hh

function f(dict<string, int> $v): varray_or_darray<arraykey, int> {
  return $v;
}

function g(dict<string, int> $v): varray_or_darray<int> {
  return $v;
}

function h(dict<string, int> $v): varray_or_darray {
  return $v;
}

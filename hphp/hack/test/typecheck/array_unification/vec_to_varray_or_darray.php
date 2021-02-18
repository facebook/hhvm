<?hh

function f(vec<int> $v): varray_or_darray<arraykey, int> {
  return $v;
}

function g(vec<int> $v): varray_or_darray<int> {
  return $v;
}

function h(vec<int> $v): varray_or_darray {
  return $v;
}

<?hh

function f(varray<int> $v): vec_or_dict<arraykey, int> {
  return $v;
}

function g(varray<int> $v): vec_or_dict<int> {
  return $v;
}

function h(varray<int> $v): vec_or_dict {
  return $v;
}

<?hh

function f(darray<string, int> $v): vec_or_dict<arraykey, int> {
  return $v;
}

function g(darray<string, int> $v): vec_or_dict<int> {
  return $v;
}

function h(darray<string, int> $v): vec_or_dict {
  return $v;
}

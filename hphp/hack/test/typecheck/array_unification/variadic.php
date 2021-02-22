<?hh

function f(int ...$as): varray<int> {
  return $as;
}

function g(int ...$as): vec<int> {
  return $as;
}

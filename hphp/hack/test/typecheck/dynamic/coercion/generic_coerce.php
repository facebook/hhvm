<?hh // strict

function foo<T>(vec<T> $v): T {
  return $v[0];
}

function bar(vec<int> $v): dynamic {
  return foo($v);
}

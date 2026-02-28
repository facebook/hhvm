<?hh

function foo(vec<dynamic> $x): vec<mixed> {
  return $x;
}

function bar(vec<mixed> $x): vec<dynamic> {
  return $x;
}

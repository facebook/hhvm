<?hh

function d(dynamic $d) : void {}

function test(Traversable<int> $e) : void {
  d(varray($e));
}

function test2(Traversable<int> $e) : void {
  $v = varray($e);
  d($v);
}

function foo<T>(vec<T> $v): T {
  return $v[0];
}

function test3(vec<int> $v): dynamic {
  return foo($v);
}

function test4(vec<int> $v): dynamic {
  $e = foo($v);
  return $e;
}

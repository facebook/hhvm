<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function test(Traversable<int> $e) : void {
  vec($e) upcast dynamic;
}

function test2(Traversable<int> $e) : void {
  $v = vec($e);
  $v upcast dynamic;
}

function foo<T>(vec<T> $v): T {
  return $v[0];
}

function test3(vec<int> $v): dynamic {
  return foo($v) upcast dynamic;
}

function test4(vec<int> $v): dynamic {
  $e = foo($v);
  return $e upcast dynamic;
}

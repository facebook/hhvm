<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>
function foo<T as (mixed...)>(int $x1, string $x2, ... T $args): void {
  var_dump($x1);
  var_dump($x2);
  var_dump($args);
}

function getVecBool(): vec<bool> {
  return vec[false];
}

<<__EntryPoint>>
function testit(): void {
  $t0 = tuple();
  $t1 = tuple(2.3);
  $t2 = tuple(false, 2.3);
  $t3 = tuple('a', false, 2.3);
  $t4 = tuple(3, 'a', false, 2.3);
  foo<(bool, float)>(3, 'a', getVecBool()[0], 2.3, ...$t0);
  foo<(bool, float)>(3, 'a', getVecBool()[0], ...$t1);
  foo<(bool, float)>(3, 'a', ...$t2);
  foo<(bool, float)>(3, ...$t3);
  foo<(bool, float)>(...$t4);
  foo(3, 'a', getVecBool()[0], 2.3, ...$t0);
  foo(3, 'a', getVecBool()[0], ...$t1);
  foo(3, 'a', ...$t2);
  foo(3, ...$t3);
  foo(...$t4);
}

<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>
function foo<T as (arraykey...)>(int $x1, string $x2, ... T $args): void {
  var_dump($x1);
  var_dump($x2);
  var_dump($args);
}

<<__EntryPoint>>
function testit(): void {
  $t2 = tuple(2, 2.3);
  $t3 = tuple('a', false, 2.3);
  $t4 = tuple(3, 'a', false, 2.3);
  foo(3, 'a', ...$t2);
  foo(3, ...$t3);
  foo(...$t4);
}

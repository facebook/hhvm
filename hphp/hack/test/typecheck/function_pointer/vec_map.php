<?hh

function foo(int $a): string {
  return '';
}

function map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}

function test(vec<int> $arg): vec<string> {
  return map($arg, foo<>);
}

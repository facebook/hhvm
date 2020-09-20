<?hh

function foo(int $a): string {
  $x = (string)$a;
  echo $x;
  return $x;
}

function map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  $return = vec[];
  foreach ($traversable as $v) {
    $return[] = $value_func($v);
  }
  return $return;
}

function test(vec<int> $arg): vec<string> {
  return map($arg, foo<>);
}

<<__EntryPoint>>
function main(): void {
  test(vec[1,2,3]);
}

<?hh


function map
  <
    Tv1,
    Tv2
  >(
  Traversable<Tv1> $traversable,
  (function (Tv1): Tv2) $value_func,
): vec<Tv2> {
  return \HH\Lib\Vec\map(
    $traversable,
    $value_func,
  );
}

function my_idx<Tk as arraykey, Tv>(?KeyedContainer<Tk, Tv> $collection, ?Tk $index): int {
  return 1;
}

function f(dict<int, Container<string>> $d, Container<mixed> $m) : void {
  map($m, $id ==> $id is arraykey ? my_idx($d, $id): 1);
}

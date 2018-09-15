<?hh // strict

<<__Rx, __OnlyRxIfArgs>>
function map<Tv1, Tv2>(
  <<__OnlyRxIfImpl(\HH\Rx\Traversable::class)>>Traversable<Tv1> $traversable,
  <<__OnlyRxIfRxFunc>>(function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  $result = vec[];
  foreach ($traversable as $value) {
    $result[] = $value_func($value);
  }
  return $result;
}

<<__Rx, __OnlyRxIfArgs>>
function func(
  <<__OnlyRxIfImpl(\HH\Rx\Traversable::class)>>Traversable<int> $traversable,
): void {
  // OK
  map($traversable, <<__Rx>> $element ==> {});
}

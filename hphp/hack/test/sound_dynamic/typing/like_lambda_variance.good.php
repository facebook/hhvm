<?hh

function my_reduce<Tv as dynamic, Ta as dynamic>(
  ~Traversable<Tv> $traversable,
  ~(function(Ta, Tv): Ta) $accumulator,
  ~Ta $initial,
) : void {}

function f(~dict<string, ?bool> $d) : void {
  my_reduce($d, ($x, $y) ==> $x ?? $y, null);
}

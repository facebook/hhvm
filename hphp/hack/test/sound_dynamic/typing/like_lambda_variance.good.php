<?hh

function my_reduce<Tv as supportdyn<mixed>, Ta as supportdyn<mixed>>(
  ~Traversable<Tv> $traversable,
  ~(function(Ta, Tv): Ta) $accumulator,
  ~Ta $initial,
) : void {}

function f(~dict<string, ?bool> $d) : void {
  my_reduce($d, ($x, $y) ==> $x ?? $y, null);
}

<?hh
function my_reduce<Tv, Ta>(
  Traversable<Tv> $traversable,
  supportdyn<(function(Ta, Tv): ~Ta)> $accumulator,
  Ta $initial,
): Ta {
  return $initial;
}

function f(
  vec<int> $components,
  int $quantity,
): ~vec<int> {
  list($_, $current) = my_reduce(
    $components,
    ($accumulator, $component) ==> {
      list($counter, $current) = $accumulator;
      $current[] = $component;
      return tuple($counter, $current);
    },
    tuple(0, vec[]),
  );
  return $current;
}

<?hh


function my_reduce<Tv as supportdyn<mixed>, Ta as supportdyn<mixed>>(
  Traversable<Tv> $traversable,
  (function(Ta, Tv): Ta) $accumulator,
  Ta $initial,
) : Ta {
  return $initial;
}

function f(supportdyn<vec<mixed>> $p): void {
  my_reduce(
    $p,
    ($result, $cp) ==> {
      $x = $cp as KeyedContainer<_, _>;
      $result[] = (string)idx($x, 'x');
      return $result;
    },
    vec[]);
}

<?hh // strict

function test(): void {
  reduce(
    vec<dict<string, mixed>>[],
    ($prev, $_) ==> {
      $key = '' as dynamic;
      if (!contains_key($prev)) {
        $prev[$key] = 0;
      }
      $prev[$key];
      return $prev;
    },
    dict[],
  );
}

function reduce<Tv, Ta>(
  Traversable<Tv> $traversable,
  (function(Ta, Tv): Ta) $accumulator,
  Ta $initial,
): Ta {
  return $initial;
}

function contains_key<Tk as arraykey>(
  KeyedContainer<Tk, mixed> $container,
): bool {
  return true;
}

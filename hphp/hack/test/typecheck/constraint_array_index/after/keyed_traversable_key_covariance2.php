<?hh

function main(): void {
  filter(
    produce(vec['a']),
    $item ==> {
      return $item['a'] === 'a'; // this should also trigger typechecker error
    },
  );
}

function produce<T>(T $t): vec<T> {
  return vec[$t];
}

function filter<Tv>(
  Traversable<Tv> $traversable,
  ?(function(Tv): bool) $value_predicate = null,
): vec<Tv> {
  return vec[];
}

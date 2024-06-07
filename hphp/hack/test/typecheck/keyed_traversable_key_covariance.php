<?hh

function main(): void {
  $vec = vec<string>['a'];
  filter(
    vec[$vec],
    (vec<string> $item) ==> {
      return $item['a'] === 'a'; //triggers typechecker error
    },
  );

  filter(
    vec[$vec],
    $item ==> {
      return $item['a'] === 'a'; // this should also trigger typechecker error
    },
  );
}

function filter<Tv>(
  Traversable<Tv> $traversable,
  ?(function(Tv): bool) $value_predicate = null,
): vec<Tv> {
  return vec[];
}

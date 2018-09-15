<?hh // strict

function filter_nulls<Tv as nonnull>(Traversable<?Tv> $traversable): vec<Tv> {
  $result = vec[];
  foreach ($traversable as $value) {
    if ($value !== null) {
      $result[] = $value;
    }
  }
  return $result;
}

function f(Traversable<mixed> $traversable): vec<nonnull> {
  return filter_nulls($traversable);
}

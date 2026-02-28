<?hh

function filter_nulls<Tv as nonnull>(Traversable<?Tv> $traversable): vec<Tv> {
  $result = vec[];
  foreach ($traversable as $value) {
    if ($value !== null) {
      $result[] = $value;
    }
  }
  return $result;
}

function f(Map<string, mixed> $m): vec<nonnull> {
  $v = vec[$m->get('foo')];
  return filter_nulls($v);
}

function wrap<T>(T $x): ?T {
  return $x;
}

function g(mixed $x): vec<nonnull> {
  $v = vec[wrap($x)];
  return filter_nulls($v);
}

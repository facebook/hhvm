<?hh // strict

<<__Rx>>
function flatten<Tv as arraykey>(
  Traversable<Traversable<Tv>> $traversables,
): keyset<Tv> {
  $result = keyset[];
  foreach ($traversables as $traversable) {
    foreach ($traversable as $value) {
      $result[] = $value;
    }
  }
  return $result;
}

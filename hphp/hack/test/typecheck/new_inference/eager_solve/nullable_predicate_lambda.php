<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

type Predicate<T> = (function(T): bool);
type KeyedPredicate<Tk, Tv> = (function(Tk, Tv): bool);

function filterWithKey<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $collection,
    KeyedPredicate<Tk, Tv> $predicate,
): Map<Tk, Tv> {
  return Map{};
}

function any<T>(
  Traversable<T> $traversable,
  ?(function(T): bool) $predicate = null,
): bool {
  return false;
}
class C { }
function groupAndFilterStatusesBySeries(
    Map<string, Map<string, Map<int, C>>> $statuses_by_series
  ): void {
    $x = filterWithKey(
      $statuses_by_series,
      ($_, $occurrence_statuses) ==> {
        return any(
        $occurrence_statuses,
        (/*Map<int, C>*/ $statuses) ==> {
          $not_confirmed_statuses = $statuses->filter(
            $status ==> false,
          );
          return false;
        }
      ); },
    );
}

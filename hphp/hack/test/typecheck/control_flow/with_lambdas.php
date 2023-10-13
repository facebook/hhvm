<?hh

class A {
  public function f(): int {
    return 0;
  }
}

function f(bool $b, Map<int, ?A> $m): void {
  if ($b) {
    return; // make a return continuation here
  }
  $m = map_compact($m); // create type variables
  $m->map(
    $a ==> $a->f(), // the return of the lambda should not unify / mess
    // up with the above created return continuation...
  );
  $m->map(
    $a ==> $a->f(), // ... otherwise this will fail.
  );
}

function map_compact<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, ?Tv> $m
): Map<Tk, Tv> {
  return new Map<Tk, Tv>(darray[]);
}

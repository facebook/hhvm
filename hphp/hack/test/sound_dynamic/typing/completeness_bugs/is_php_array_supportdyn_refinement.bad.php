<?hh

function f<Tk as arraykey, Tv as supportdyn<nonnull> >(
KeyedTraversable<Tk, ?Tv> $traversable,
): void {
}

function testit(supportdyn<mixed> $m):void {
  invariant(HH\is_php_array($m), 'hack');
  f($m);
}

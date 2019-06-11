<?hh // strict

<<__Rx>>
function f1(array<int> $a): void {
  // OK
  foreach ($a as $c) {
  }
}

<<__Rx>>
function f2(\HH\Rx\Traversable<int> $a): void {
  // OK
  foreach ($a as $c) {
  }
}

<<__Rx>>
function f3(\HH\Rx\KeyedTraversable<int, int> $a): void {
  // OK
  foreach ($a as $c) {
  }
}

<<__Rx>>
function f4(varray<int> $a): void {
  // OK
  foreach ($a as $c) {
  }
}

<<__Rx>>
function f5(darray<int, int> $a): void {
  // OK
  foreach ($a as $c) {
  }
}

<<__Rx, __AtMostRxAsArgs>>
function f6(
  <<__OnlyRxIfImpl(\HH\Rx\Traversable::class)>>Traversable<int> $a,
): void {
  // OK
  foreach ($a as $c) {
  }
}

<<__Rx>>
function f7<T as \HH\Rx\Traversable<int>>(T $a): void {
  // OK
  foreach ($a as $c) {
  }
}

<<__Rx>>
function toArray(Iterable<int> $a): array<int> {
  throw new Exception();
}

<<__Rx, __AtMostRxAsArgs>>
function f8(
  <<__OnlyRxIfImpl(\HH\Rx\Traversable::class)>>Traversable<int> $a,
): void {
  if ($a is Iterable<_>) {
    $a = toArray($a);
  }
  // OK, $a is either array or iterable
  foreach ($a as $c) {
  }
}

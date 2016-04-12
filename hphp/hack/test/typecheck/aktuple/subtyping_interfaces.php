<?hh //strict

/**
 * Tuple-like arrays are subtypes of all the special interfaces that arrays are
 * too.
 */

function test(): void {
  $a = array('aaa', 4);

  take_traversable($a);
  take_container($a);
  take_keyed_traversable($a);
  take_keyed_container($a);
  take_indexish($a);
}

function take_array<Tk, Tv>(array<Tk, Tv> $a): void {}
function take_traversable<Tv>(Traversable<Tv> $a): void {}
function take_container<Tv>(Container<Tv> $a): void {}
function take_keyed_traversable<Tk, Tv>(KeyedTraversable<Tk, Tv> $a): void {}
function take_keyed_container<Tk, Tv>(KeyedContainer<Tk, Tv> $a): void {}
function take_indexish<Tk, Tv>(Indexish<Tk, Tv> $a): void {}

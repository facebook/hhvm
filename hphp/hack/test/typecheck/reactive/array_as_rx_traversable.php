<?hh // strict

<<__Rx>>
function f1(\HH\Rx\Traversable<int> $coll): void {
}

<<__Rx>>
function f2(\HH\Rx\KeyedTraversable<int, int> $coll): void {
}

<<__Rx>>
function f(): void {
  f1(array(1, 2, 3));
  f2(array(1 => 1, 2 => 2));
  f1(array());
  f2(array());
  f1(varray[1]);
  f2(darray[1 => 1]);
}

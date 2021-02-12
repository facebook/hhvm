<?hh // strict

function f1(\HH\Rx\Traversable<int> $coll): void {
}


function f2(\HH\Rx\KeyedTraversable<int, int> $coll): void {
}


function f(): void {
  f1(varray[1, 2, 3]);
  f2(darray[1 => 1, 2 => 2]);
  f1(varray[]);
  f2(varray[]);
  f1(varray[1]);
  f2(darray[1 => 1]);
}

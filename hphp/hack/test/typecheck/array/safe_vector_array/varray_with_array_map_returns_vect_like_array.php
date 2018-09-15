<?hh // strict

function providesVectLikeArray(): array<int> {
  return array_map($x ==> $x, varray[1, 2, 3, 4, 5]);
}

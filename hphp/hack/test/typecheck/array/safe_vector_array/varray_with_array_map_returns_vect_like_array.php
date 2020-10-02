<?hh // strict

function providesVectLikeArray(): varray<int> {
  return array_map($x ==> $x, varray[1, 2, 3, 4, 5]);
}

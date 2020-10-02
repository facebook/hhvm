<?hh // partial

function providesVectLikeArray(): varray<int> {
  return array_filter(varray[1, 2, 3, 4, 5], $x ==> true);
}

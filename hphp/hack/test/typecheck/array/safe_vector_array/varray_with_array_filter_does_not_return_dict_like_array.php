<?hh // partial

function providesDictLikeArray(): darray<int, int> {
  return array_filter(varray[1, 2, 3, 4, 5], $x ==> true);
}

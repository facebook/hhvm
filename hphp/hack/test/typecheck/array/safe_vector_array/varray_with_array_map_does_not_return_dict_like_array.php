<?hh // strict

function providesDictLikeArray(): darray<int, int> {
  return array_map($x ==> $x, varray[1, 2, 3, 4, 5]);
}

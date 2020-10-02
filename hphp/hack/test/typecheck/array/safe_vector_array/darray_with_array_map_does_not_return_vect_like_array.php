<?hh // strict

function providesDictLikeArray(): varray<bool> {
  return array_map($x ==> $x, darray["foo" => true, "bar" => false]);
}

<?hh // partial

function providesDictLikeArray(): varray<bool> {
  return array_filter(darray["foo" => true, "bar" => false], $x ==> true);
}

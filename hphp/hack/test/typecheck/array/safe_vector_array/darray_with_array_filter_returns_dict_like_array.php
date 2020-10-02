<?hh // partial

function providesDictLikeArray(): darray<string, bool> {
  return array_filter(darray["foo" => true, "bar" => false], $x ==> true);
}

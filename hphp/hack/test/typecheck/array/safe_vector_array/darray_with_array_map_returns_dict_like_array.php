<?hh // strict

function providesDictLikeArray(): darray<string, bool> {
  return array_map($x ==> $x, darray["foo" => true, "bar" => false]);
}

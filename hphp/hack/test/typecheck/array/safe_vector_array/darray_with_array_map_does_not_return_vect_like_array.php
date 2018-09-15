<?hh // strict

function providesDictLikeArray(): array<bool> {
  return array_map($x ==> $x, darray["foo" => true, "bar" => false]);
}

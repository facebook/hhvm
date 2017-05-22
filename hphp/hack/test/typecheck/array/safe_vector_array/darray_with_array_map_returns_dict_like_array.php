<?hh // strict

function providesDictLikeArray(): array<string, bool> {
  return array_map($x ==> $x, darray["foo" => true, "bar" => false]);
}

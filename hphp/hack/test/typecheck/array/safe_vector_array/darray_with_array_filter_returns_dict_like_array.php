<?hh // partial

function providesDictLikeArray(): array<string, bool> {
  return array_filter(darray["foo" => true, "bar" => false], $x ==> true);
}

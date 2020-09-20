<?hh // partial

function providesDictLikeArray(): array<bool> {
  return array_filter(darray["foo" => true, "bar" => false], $x ==> true);
}

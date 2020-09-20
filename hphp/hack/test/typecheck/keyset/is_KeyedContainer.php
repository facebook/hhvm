<?hh // partial

function foo(keyset<string> $k): KeyedContainer<string, string> {
  return $k;
}

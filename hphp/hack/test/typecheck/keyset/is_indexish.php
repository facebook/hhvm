<?hh

function foo(keyset<string> $k): Indexish<string, string> {
  return $k;
}

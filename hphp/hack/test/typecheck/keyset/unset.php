<?hh // partial

function foo(keyset<int> $k): void {
  unset($k[42]);
}

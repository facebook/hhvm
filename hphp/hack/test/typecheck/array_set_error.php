<?hh // partial

function foo(keyset<int> $k): void {
  $k[10] = 10;
}

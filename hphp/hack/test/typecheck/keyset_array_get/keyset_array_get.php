<?hh

function test_keyset(keyset<string> $ks): string {
  return $ks["foo"];  // Should warn
}

function test_keyset_variable(keyset<int> $ks, int $key): int {
  return $ks[$key];  // Should warn
}

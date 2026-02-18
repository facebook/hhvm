<?hh

function test_keyset(keyset<string> $ks, string $key): void {
  $_ = $ks[$key];
  //      ^ at-caret
}

<?hh

function test_like_keyset(~keyset<string> $ks): ~string {
  return $ks["foo"];  // Should warn - underlying type is keyset
}

function test_like_keyset_variable(~keyset<int> $ks, int $key): ~int {
  return $ks[$key];  // Should warn - underlying type is keyset
}

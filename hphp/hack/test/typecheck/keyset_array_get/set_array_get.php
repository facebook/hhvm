<?hh

function test_set(Set<string> $s): string {
  return $s["foo"]; // Should warn (ConstSet subtype)
}

function test_immset(ImmSet<string> $s): string {
  return $s["bar"]; // Should warn (ConstSet subtype)
}

function test_constset(ConstSet<string> $s): string {
  return $s["baz"]; // Should warn
}

function test_mutableset(MutableSet<string> $s): string {
  return $s["qux"]; // Should warn (ConstSet subtype)
}

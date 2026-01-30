<?hh

function test_dict(dict<string, int> $d): int {
  return $d["foo"]; // No warning - dict is fine
}

function test_vec(vec<int> $v): int {
  return $v[0]; // No warning - vec is fine
}

function test_Map(Map<int, int> $v): int {
  return $v[0]; // No warning - Map is fine
}

function test_append(keyset<int> $ks, Set<int> $s): void {
  $ks[] = 1; // No warning - we can append
  $s[] = 1; // No warning - we can append
}

function test_keyset_unset(keyset<int> $ks): void {
  unset($ks[0]); // No warning - valid unset operation
}

function test_isset_unset(keyset<int> $ks): void {
  isset($ks[0]); // No warning - valid isset operation
}

function test_assign(keyset<int> $ks, Set<int> $s): void {
  $ks[1] = 1; // No warning - this is an error insted
  $s[1] = 1; // No warning - this is an error insted
}

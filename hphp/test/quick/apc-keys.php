<?hh

<<__EntryPoint>>
function main(): void {
  // Create a dict with non-static string keys that happen to be in the static
  // string lookup table. When we add this dict to APC, they'll become static.
  $keys = vec['foo', 'bar'];
  $x = dict[];
  foreach ($keys as $key) {
    $x[$key[0] . $key[1] . $key[2]] = true;
  }
  apc_add('apc-key', $x);

  // Verify that a COW of $x with more non-static strings keys is okay.
  $success = null;
  $y = apc_fetch('apc-key', inout $success);
  $y[json_decode('"asdf"')] = true;
  var_dump($y);
}

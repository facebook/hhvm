<?hh

function check(string $path, ?string $expected): void {
  $package = HH\Facts\path_to_package($path);
  $actual = $package ?? 'null';
  $exp = $expected ?? 'null';
  if ($actual === $exp) {
    echo "OK $path => $actual\n";
  } else {
    echo "FAIL $path => $actual (expected $exp)\n";
  }
}

<<__EntryPoint>>
function main(): void {
  // Longest prefix wins: flib/prod/ matches "prod", not "flib" or "fallthrough"
  check('flib/prod/deep.inc', 'prod');

  // Shorter prefix when no deeper match: flib/other/ matches "flib"
  check('flib/other/shallow.inc', 'flib');

  // Catch-all fallthrough: other/ matches "fallthrough"
  check('other/unmatched.inc', 'fallthrough');

  // Regression: short name "a" with long prefix beats long name "fallthrough"
  check('subdir/no-override.inc', 'a');

  // Multiple include paths per package
  check('multi_a/file.inc', 'multi');
  check('multi_b/file.inc', 'multi');

  // __PackageOverride takes precedence over path-based matching
  check('subdir/with-override.inc', 'custom');

  // Non-existent file returns null
  check('does/not/exist.php', null);
}

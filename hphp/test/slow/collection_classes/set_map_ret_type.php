<?hh

// Test that Set::map() throws if the return type of its callback is invalid.
// Only int and string are currently allowed.

function main() {

  $s = Set {1, 2, 3};

  // int
  $s1 = $s->map(function ($v) { return $v + 1; });
  var_dump($s1 == Set {2, 3, 4});

  // string
  $s2 = $s->map(function ($v) { return strval($v); });
  var_dump($s2 == Set {"1", "2", "3"});

  // invalid (should throw)
  $s3 = $s->map(function ($v) { return Pair {$v, $v}; });
}

main();

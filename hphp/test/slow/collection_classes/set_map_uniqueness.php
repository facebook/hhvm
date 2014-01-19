<?hh

// Check that Set::map() removes duplicates from the resulting set.

function main() {
  $s1 = Set {1, 2, 3};
  $s2 = $s1->map(function ($v) { return 42; });

  var_dump($s2->count());
  foreach ($s2 as $e) {
    var_dump($e);
  }
}

main();

<?hh

// Check that literal syntax works for FixedSet.

function main() {
  $fs = FixedSet {1, 2, 3};
  foreach (Vector {1, 2, 3} as $e) {
    var_dump($fs->contains($e));
  }
}

main();

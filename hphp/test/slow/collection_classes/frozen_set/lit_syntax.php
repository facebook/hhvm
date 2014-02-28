<?hh

// Check that literal syntax works for ImmSet.

function main() {
  $fs = ImmSet {1, 2, 3};
  foreach (Vector {1, 2, 3} as $e) {
    var_dump($fs->contains($e));
  }
}

main();

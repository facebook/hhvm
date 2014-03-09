<?hh

// Test that FS is immutable.

function main() {
  $fs = ImmSet {1, 2, 3};
  $fs->add(2);
}

main();

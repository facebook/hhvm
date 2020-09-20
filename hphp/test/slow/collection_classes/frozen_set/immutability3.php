<?hh

// Test that FS is immutable.

function main() {
  $fs = ImmSet {1, 2, 3};
  $fs->addAll(Vector {1, 2, 3});
}


<<__EntryPoint>>
function main_immutability3() {
main();
}

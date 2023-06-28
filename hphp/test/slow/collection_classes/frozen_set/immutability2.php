<?hh

// Test that FS is immutable.

function main() :mixed{
  $fs = ImmSet {1, 2, 3};
  $fs->add(2);
}


<<__EntryPoint>>
function main_immutability2() :mixed{
main();
}

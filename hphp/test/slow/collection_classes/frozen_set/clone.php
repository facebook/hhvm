<?hh

// Test that we can clone a ImmSet.

function main() {
  $fs = ImmSet {1, 2, 3};
  $clone = clone $fs;
  var_dump($clone == $fs);
}


<<__EntryPoint>>
function main_clone() {
main();
}

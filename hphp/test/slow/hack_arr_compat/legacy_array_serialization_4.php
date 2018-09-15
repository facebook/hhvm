<?hh

// weird corner case--we need to guarantee the passed-in vec/dict isn't changed
// by // producing a legacy-behaving copy of it
function test_cow_vec() {
  $a = vec[1, 2, 3];
  echo serialize($a) . "\n";
  HH\enable_legacy_behavior($a);
  echo serialize($a) . "\n";
}


<<__EntryPoint>>
function main_legacy_array_serialization_4() {
test_cow_vec();
}

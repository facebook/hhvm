<?hh

function get_dict() : dict<int, int>{
  return __hhvm_intrinsics\launder_value(dict [
    0 => 5,
  ]);
}

function get_maybe_int() {
  return __hhvm_intrinsics\launder_value(1);
}

function getSortedValue(?dict<int, int> $d = null) : int {
  return idx($d, get_maybe_int(), 5);
}

function test($d) {
  return getSortedValue($d);
}
__hhvm_intrinsics\disable_inlining('get_dict');
__hhvm_intrinsics\disable_inlining('test');

function main() {
  // Generate a profile with a null dict.
  getSortedValue();
  $d = get_dict();
  for ($i = 0; $i < 50; ++$i) {
    // Run a bunch with a guaranteed non null dict.
    test($d);
  }
}

main();
echo "Success";

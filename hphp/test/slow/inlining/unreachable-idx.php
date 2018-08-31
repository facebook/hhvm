<?hh

<<__NEVER_INLINE>>
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

<<__NEVER_INLINE>>
function test($d) {
  return getSortedValue($d);
}

function main() {
  // Generate a profile with a null dict.
  getSortedValue();
  $d = get_dict();
  for ($i = 0; $i < 50; ++$i) {
    // Run a bunch with a guaranteed non null dict.
    test($d);
  }
}


<<__EntryPoint>>
function main_unreachable_idx() {
main();
echo "Success";
}

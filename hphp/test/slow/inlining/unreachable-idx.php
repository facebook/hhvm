<?hh

<<__NEVER_INLINE>>
function get_dict() : dict<int, int>{
  return __hhvm_intrinsics\launder_value(dict [
    0 => 5,
  ]);
}

function get_maybe_int() :mixed{
  return __hhvm_intrinsics\launder_value(1);
}

function getSortedValue(?dict<int, int> $d = null) : int {
  return idx($d, get_maybe_int(), 5);
}

<<__NEVER_INLINE>>
function test($d) :mixed{
  return getSortedValue($d);
}

function main() :mixed{
  // Generate a profile with a null dict.
  getSortedValue();
  $d = get_dict();
  for ($i = 0; $i < 50; ++$i) {
    // Run a bunch with a guaranteed non null dict.
    test($d);
  }
}


<<__EntryPoint>>
function main_unreachable_idx() :mixed{
main();
echo "Success";
}

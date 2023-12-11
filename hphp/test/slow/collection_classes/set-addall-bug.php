<?hh
function main() :mixed{
  // This Set will have an initial capacity of 3
  $x = HH\Set {205};
  // This array has 25 elements, but only 3 unique values
  $y = vec[206, 206, 206, 206, 206, 207, 207, 207, 207, 207, 207, 207, 207,
             207, 207, 207, 207, 207, 206, 206, 206, 206, 207, 221, 206];
  // addAll() will speculatively increase capacity to accommodate 25+1 = 26
  // elements. Set always chooses capacities that are 3 times a power of two,
  // so it will grow to a capacity of 48. After all the elements have been
  // added (throwing away duplicates) there will only be 4 elements, and
  // 4/48 = 1/12 is sufficiently small to trigger addAll() to shrink the
  // capacity. There was a bug where capacity was being decreased to the old
  // capacity (3), which is not large enough to accommodate the current number
  // of elements (4).
  $x->addAll($y);
  foreach ($x as $v) {
    echo "$v\n";
  }
}

<<__EntryPoint>>
function main_set_addall_bug() :mixed{
main();
}

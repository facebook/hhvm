<?hh

<<__EntryPoint>>
function main_compare() {
  $x = darray[];
  $y = __hhvm_intrinsics\dummy_cast_to_kindofarray($x);
  var_dump($x == $y);
  var_dump($x === $y);
  var_dump($y < $y);
  var_dump($y <= $y);
  var_dump($y > $y);
  var_dump($y >= $y);
}

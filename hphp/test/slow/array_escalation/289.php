<?hh


<<__EntryPoint>>
function main_289() {
  $a = darray[0 => 10];
  $a[2] = varray[0];
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray[10]);
  $a[2] = varray[0];
  var_dump($a);
}

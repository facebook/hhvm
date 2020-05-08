<?hh


<<__EntryPoint>>
function main_288() {
  $a = darray[0 => 10];
  $a[2] = 'test';
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray[10]);
  $a[2] = 'test';
  var_dump($a);
}

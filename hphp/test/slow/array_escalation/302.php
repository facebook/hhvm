<?hh


<<__EntryPoint>>
function main_302() {
  $a = darray[0 => 'test'];
  $a[2] = 1;
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray['test']);
  $a[2] = 1;
  var_dump($a);
}

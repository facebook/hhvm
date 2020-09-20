<?hh


<<__EntryPoint>>
function main_303() {
  $a = darray[0 => 'test'];
  $a[2] = 'test';
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray['test']);
  $a[2] = 'test';
  var_dump($a);
}

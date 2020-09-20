<?hh


<<__EntryPoint>>
function main_291() {
  $a = darray[0 => 10];
  $a['test'] = 'test';
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray[10]);
  $a['test'] = 'test';
  var_dump($a);
}

<?hh


<<__EntryPoint>>
function main_290() {
  $a = darray[0 => 10];
  $a['test'] = 1;
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray[10]);
  $a['test'] = 1;
  var_dump($a);
}

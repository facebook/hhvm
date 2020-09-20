<?hh


<<__EntryPoint>>
function main_306() {
  $a = darray[0 => 'test'];
  $a['test'] = 'test';
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray['test']);
  $a['test'] = 'test';
  var_dump($a);
}

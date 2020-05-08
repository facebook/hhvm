<?hh


<<__EntryPoint>>
function main_305() {
  $a = darray[0 => 'test'];
  $a['test'] = 1;
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray['test']);
  $a['test'] = 1;
  var_dump($a);
}

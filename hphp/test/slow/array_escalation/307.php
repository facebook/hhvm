<?hh


<<__EntryPoint>>
function main_307() {
  $a = darray[0 => 'test'];
  $a['test'] = varray[0];
  var_dump($a);

  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray['test']);
  $a['test'] = varray[0];
  var_dump($a);
}

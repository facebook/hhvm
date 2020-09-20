<?hh


<<__EntryPoint>>
function main_209() {
  $a = varray[1];
  $a[] = 3;
  var_dump($a);

  $a = darray[0 => 1];
  $a['test'] = 3;
  var_dump($a);
}

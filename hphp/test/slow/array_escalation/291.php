<?hh


<<__EntryPoint>>
function main_291() {
  $a = darray[0 => 10];
  $a['test'] = 'test';
  var_dump($a);

  $a = darray(varray[10]);
  $a['test'] = 'test';
  var_dump($a);
}

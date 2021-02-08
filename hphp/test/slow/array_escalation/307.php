<?hh


<<__EntryPoint>>
function main_307() {
  $a = darray[0 => 'test'];
  $a['test'] = varray[0];
  var_dump($a);

  $a = darray(varray['test']);
  $a['test'] = varray[0];
  var_dump($a);
}

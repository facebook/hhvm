<?hh


<<__EntryPoint>>
function main_306() :mixed{
  $a = darray[0 => 'test'];
  $a['test'] = 'test';
  var_dump($a);

  $a = darray(varray['test']);
  $a['test'] = 'test';
  var_dump($a);
}

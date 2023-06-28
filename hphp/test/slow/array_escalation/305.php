<?hh


<<__EntryPoint>>
function main_305() :mixed{
  $a = darray[0 => 'test'];
  $a['test'] = 1;
  var_dump($a);

  $a = darray(varray['test']);
  $a['test'] = 1;
  var_dump($a);
}

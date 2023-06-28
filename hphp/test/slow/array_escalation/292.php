<?hh


<<__EntryPoint>>
function main_292() :mixed{
  $a = darray[0 => 10];
  $a['test'] = varray[0];
  var_dump($a);

  $a = darray(varray[10]);
  $a['test'] = varray[0];
  var_dump($a);
}

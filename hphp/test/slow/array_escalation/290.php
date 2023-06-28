<?hh


<<__EntryPoint>>
function main_290() :mixed{
  $a = darray[0 => 10];
  $a['test'] = 1;
  var_dump($a);

  $a = darray(varray[10]);
  $a['test'] = 1;
  var_dump($a);
}

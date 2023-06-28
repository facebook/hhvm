<?hh


<<__EntryPoint>>
function main_302() :mixed{
  $a = darray[0 => 'test'];
  $a[2] = 1;
  var_dump($a);

  $a = darray(varray['test']);
  $a[2] = 1;
  var_dump($a);
}

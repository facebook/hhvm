<?hh


<<__EntryPoint>>
function main_288() :mixed{
  $a = darray[0 => 10];
  $a[2] = 'test';
  var_dump($a);

  $a = darray(varray[10]);
  $a[2] = 'test';
  var_dump($a);
}

<?hh


<<__EntryPoint>>
function main_210() :mixed{
  $a = varray[1, 2];
  $a[] = 3;
  var_dump($a);

  $a = darray[0 => 1, 1 => 2];
  $a[10] = 3;
  var_dump($a);
}

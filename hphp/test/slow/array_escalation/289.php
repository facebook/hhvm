<?hh


<<__EntryPoint>>
function main_289() :mixed{
  $a = darray[0 => 10];
  $a[2] = varray[0];
  var_dump($a);

  $a = darray(varray[10]);
  $a[2] = varray[0];
  var_dump($a);
}

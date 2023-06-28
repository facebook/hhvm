<?hh


<<__EntryPoint>>
function main_303() :mixed{
  $a = darray[0 => 'test'];
  $a[2] = 'test';
  var_dump($a);

  $a = darray(varray['test']);
  $a[2] = 'test';
  var_dump($a);
}

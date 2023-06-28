<?hh


<<__EntryPoint>>
function main_304() :mixed{
  $a = darray[0 => 'test'];
  $a[2] = varray[0];
  var_dump($a);

  $a = darray(varray['test']);
  $a[2] = varray[0];
  var_dump($a);
}

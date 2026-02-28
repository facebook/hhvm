<?hh


<<__EntryPoint>>
function main_302() :mixed{
  $a = dict[0 => 'test'];
  $a[2] = 1;
  var_dump($a);

  $a = darray(vec['test']);
  $a[2] = 1;
  var_dump($a);
}

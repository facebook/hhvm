<?hh


<<__EntryPoint>>
function main_303() :mixed{
  $a = dict[0 => 'test'];
  $a[2] = 'test';
  var_dump($a);

  $a = darray(vec['test']);
  $a[2] = 'test';
  var_dump($a);
}

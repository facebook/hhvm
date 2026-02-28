<?hh


<<__EntryPoint>>
function main_306() :mixed{
  $a = dict[0 => 'test'];
  $a['test'] = 'test';
  var_dump($a);

  $a = darray(vec['test']);
  $a['test'] = 'test';
  var_dump($a);
}

<?hh


<<__EntryPoint>>
function main_305() :mixed{
  $a = dict[0 => 'test'];
  $a['test'] = 1;
  var_dump($a);

  $a = darray(vec['test']);
  $a['test'] = 1;
  var_dump($a);
}

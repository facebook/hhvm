<?hh


<<__EntryPoint>>
function main_307() :mixed{
  $a = dict[0 => 'test'];
  $a['test'] = vec[0];
  var_dump($a);

  $a = darray(vec['test']);
  $a['test'] = vec[0];
  var_dump($a);
}

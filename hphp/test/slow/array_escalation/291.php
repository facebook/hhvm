<?hh


<<__EntryPoint>>
function main_291() :mixed{
  $a = dict[0 => 10];
  $a['test'] = 'test';
  var_dump($a);

  $a = darray(vec[10]);
  $a['test'] = 'test';
  var_dump($a);
}

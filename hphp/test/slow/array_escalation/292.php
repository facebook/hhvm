<?hh


<<__EntryPoint>>
function main_292() :mixed{
  $a = dict[0 => 10];
  $a['test'] = vec[0];
  var_dump($a);

  $a = darray(vec[10]);
  $a['test'] = vec[0];
  var_dump($a);
}

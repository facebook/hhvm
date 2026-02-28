<?hh


<<__EntryPoint>>
function main_209() :mixed{
  $a = vec[1];
  $a[] = 3;
  var_dump($a);

  $a = dict[0 => 1];
  $a['test'] = 3;
  var_dump($a);
}

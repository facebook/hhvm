<?hh


<<__EntryPoint>>
function main_290() :mixed{
  $a = dict[0 => 10];
  $a['test'] = 1;
  var_dump($a);

  $a = darray(vec[10]);
  $a['test'] = 1;
  var_dump($a);
}

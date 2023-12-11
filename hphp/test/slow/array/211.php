<?hh


<<__EntryPoint>>
function main_211() :mixed{
  $a = dict[0 => 1, 1 => 2];
  $a['10'] = 3;
  var_dump($a);
}

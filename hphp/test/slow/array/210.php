<?hh


<<__EntryPoint>>
function main_210() :mixed{
  $a = vec[1, 2];
  $a[] = 3;
  var_dump($a);

  $a = dict[0 => 1, 1 => 2];
  $a[10] = 3;
  var_dump($a);
}

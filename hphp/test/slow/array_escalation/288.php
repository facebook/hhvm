<?hh


<<__EntryPoint>>
function main_288() :mixed{
  $a = dict[0 => 10];
  $a[2] = 'test';
  var_dump($a);

  $a = darray(vec[10]);
  $a[2] = 'test';
  var_dump($a);
}

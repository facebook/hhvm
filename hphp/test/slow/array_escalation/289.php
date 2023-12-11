<?hh


<<__EntryPoint>>
function main_289() :mixed{
  $a = dict[0 => 10];
  $a[2] = vec[0];
  var_dump($a);

  $a = darray(vec[10]);
  $a[2] = vec[0];
  var_dump($a);
}

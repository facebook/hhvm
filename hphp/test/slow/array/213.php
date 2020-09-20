<?hh


<<__EntryPoint>>
function main_213() {
  $a = darray[0 => 1, 1 => 2];
  $b = $a;
  $a['10'] = 3;
  var_dump($a);
}

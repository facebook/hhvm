<?hh


<<__EntryPoint>>
function main_211() {
  $a = darray[0 => 1, 1 => 2];
  $a['10'] = 3;
  var_dump($a);
}

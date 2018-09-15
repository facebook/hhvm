<?hh

function main($a, $b) {
  $d = dict[$a => &$b];
  var_dump($d);
}


<<__EntryPoint>>
function main_init_parse_error4() {
main(1, 2);
}

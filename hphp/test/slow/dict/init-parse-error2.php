<?hh

function main($a, $b) {
  $d = dict[&$a => 1, &$b => 2];
  var_dump($d);
}


<<__EntryPoint>>
function main_init_parse_error2() {
main(1, 2);
}

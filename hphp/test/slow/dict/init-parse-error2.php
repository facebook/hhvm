<?hh

function main($a, $b) {
  $d = dict[&$a => 1, &$b => 2];
  var_dump($d);
}

main(1, 2);

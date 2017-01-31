<?hh

function main($a, $b) {
  $d = dict[$a => &$b];
  var_dump($d);
}

main(1, 2);

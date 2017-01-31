<?hh

function main($a) {
  $d = dict[&$a, 1, 2, 3];
  var_dump($d);
}

main(1);

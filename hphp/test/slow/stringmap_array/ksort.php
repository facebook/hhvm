<?hh

function main() {
  $a = msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;

  ksort($a);
  var_dump($a);
}

main();

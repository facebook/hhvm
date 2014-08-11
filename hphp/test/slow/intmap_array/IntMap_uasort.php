<?hh

function main() {
  $a = miarray();
  $a[1] = 2;
  $a[500] = 0;
  var_dump($a);
  uasort($a, function($x, $y) {
      if ($x < $y) {
        return -1;
      } else {
        return 1;
      }
    });
  var_dump($a);
}

main();

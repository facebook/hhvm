<?hh
function main() {
  $g = $GLOBALS;
  foreach (array_keys($GLOBALS) as $k) {
    unset($GLOBALS[$k]);
  }

  var_dump($g);
  var_dump((bool)$g);
}

main();

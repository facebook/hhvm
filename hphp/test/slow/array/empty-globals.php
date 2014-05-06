<?hh
function main() {
  $g = $GLOBALS;
  foreach (array_keys($GLOBALS) as $k) {
    unset($GLOBALS[$k]);
  }

  var_dump($g);
  // This will say "true", which is intentionally divergent from PHP5.
  var_dump((bool)$g);
}

main();

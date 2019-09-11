<?hh
function main() {
  $g = $GLOBALS['GLOBALS'];
  foreach (HH\global_keys() as $k) {
    unset($GLOBALS[$k]);
  }

  var_dump($g);
  var_dump((bool)$g);
}


<<__EntryPoint>>
function main_empty_globals() {
main();
}

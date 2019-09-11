<?hh

$a = 100;

function f() {
  foreach (HH\global_keys() as $k) {
    if ($k == 'a') {
      $GLOBALS[$k] = -1;
    }
  }

  var_dump($GLOBALS['a']);
  $b = $GLOBALS['GLOBALS'];
  $b['a'] = 0;
  var_dump($GLOBALS['a']);
}
f();

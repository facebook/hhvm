<?hh

class obj {
  function __toString() {
    if ($GLOBALS['x']++ == 1) {
      unset($GLOBALS['a']);
    }
    return 'obj';
  }
}

function foo($x) {
  return $x['a'] . array_key_exists('a', $x). $x['a'] .
    array_key_exists('a', $x);
}
<<__EntryPoint>>
function entrypoint_globals2(): void {

  for ($i = 0; $i < 100; ++$i) {
    $GLOBALS['x'] = 0;
    $GLOBALS['a'] = new obj;
    var_dump(foo($GLOBALS['GLOBALS']));
  }
}

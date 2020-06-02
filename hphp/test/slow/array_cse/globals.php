<?hh

class obj {
  function __toString() {
    if ($GLOBALS['x']++ == 1) {
      $GLOBALS['a'] = 'asd';
    }
    return 'obj';
  }
}

function foo($x) {
  return $x['a'] . $x['a'] . $x['a'];
}
<<__EntryPoint>>
function entrypoint_globals(): void {

  for ($i = 0; $i < 100; ++$i) {
    $GLOBALS['x'] = 0;
    $GLOBALS['a'] = new obj;
    var_dump(foo($GLOBALS['GLOBALS']));
  }
}

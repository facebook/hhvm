<?php

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

for ($i = 0; $i < 100; ++$i) {
  $x = 0;
  $a = new obj;
  var_dump(foo($GLOBALS));
}

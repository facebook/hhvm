<?php

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

for ($i = 0; $i < 100; ++$i) {
  $x = 0;
  $a = new obj;
  var_dump(foo($GLOBALS));
}

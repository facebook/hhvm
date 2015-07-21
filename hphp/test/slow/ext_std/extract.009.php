<?php

class X {
  function __destruct() {
    $GLOBALS['V'] = "Oops";
  }
}

function main() {
  $o = new X;
  $x =& $GLOBALS['V'];
  $x = array('o' => null, 'y' => null);
  extract($x, EXTR_REFS);
  var_dump(get_defined_vars());
}

main();

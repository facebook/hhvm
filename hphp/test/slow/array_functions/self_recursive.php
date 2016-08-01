<?php

function test($g) {
  $GLOBALS['g'] = $GLOBALS;

  array_replace_recursive($GLOBALS, $g);

  $GLOBALS['g'] = $GLOBALS;
  array_merge_recursive($GLOBALS, $g);
}

function main() {
  $a = array();
  $a['g'] = &$a;

  test($a);
  test($GLOBALS);
  var_dump(compact($a));
}

main();

<?php

function test($g) {
  $GLOBALS['g'] = $GLOBALS;

  array_replace_recursive($GLOBALS, $g);

  $GLOBALS['g'] = $GLOBALS;
  array_merge_recursive($GLOBALS, $g);
}

function main() {
  $a = array('g' => $GLOBALS);

  test($a);
  test($GLOBALS);
}


<<__EntryPoint>>
function main_self_recursive() {
main();
}

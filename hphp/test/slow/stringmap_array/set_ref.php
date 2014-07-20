<?php

function cow_setRef($arr) {
  $foo = array();
  $arr['foo'] = &$foo; // warning
}

function main() {
  $foo = array();
  $a = hphp_msarray();
  $a['foo'] = &$foo; // warning

  $a = hphp_msarray();
  cow_setRef($a);
  $a[] = "warning"; // warning
}

main();

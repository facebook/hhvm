<?php

function cow_appendRef($arr) {
  $foo = "foo";
  $arr[] = &$foo; // warning
}

function main() {
  $foo = "foo";
  $a = hphp_msarray();
  $a[] = &$foo; // warning
  $a[] = &$foo; // no warning

  $a = hphp_msarray();
  cow_appendRef($a);
  $a[] = &$foo; // no warning
}

main();

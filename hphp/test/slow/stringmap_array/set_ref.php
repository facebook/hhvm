<?hh

function cow_setRef($arr) {
  $foo = array();
  $arr['foo'] = &$foo; // warning
}

function main() {
  $foo = array();
  $a = msarray();
  $a['foo'] = &$foo; // warning

  $a = msarray();
  cow_setRef($a);
  $a[] = "warning"; // warning
}

main();

<?hh

function cow_appendRef($arr) {
  $foo = "foo";
  $arr[] = &$foo; // warning
}

function main() {
  $foo = "foo";
  $a = msarray();
  $a[] = &$foo; // warning
  $a[] = &$foo; // no warning

  $a = msarray();
  cow_appendRef($a);
  $a[] = &$foo; // no warning
}

main();

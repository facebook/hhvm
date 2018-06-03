<?hh

function bar($a) {
  var_dump($a);
  var_dump($GLOBALS['FOO']);
}

function foo(inout $x) {
  $x = 20;
  bar($x);
}

function main() {
  $x = 10;
  $GLOBALS['FOO'] =& $x;
  bar($x);
  foo(&$x);
  bar($x);
}

main();

<?

<<__NEVER_INLINE>>
function foo() {
  return 4;
}

<<__NEVER_INLINE>>
function bar($x) {
  echo var_dump($x[3]);
}

function main() {
  $a = array(1, 2, 3, foo());
  bar($a);
}

main();

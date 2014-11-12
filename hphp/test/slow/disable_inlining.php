<?

function foo() {
  return 4;
}

function bar($x) {
  echo var_dump($x[3]);
}

function main() {
  $a = array(1, 2, 3, foo());
  bar($a);
}

if (is_callable('__hhvm_intrinsics\disable_inlining')) {
  __hhvm_intrinsics\disable_inlining('foo');
  __hhvm_intrinsics\disable_inlining('bar');
}

main();

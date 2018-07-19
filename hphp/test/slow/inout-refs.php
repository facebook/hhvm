<?hh

function foo(inout $x) {
  $x = 12;
}

function main() {
  $a = ['a', [3, 1, 2], 'b'];
  sort(&$a[1]);
  foo(inout $a[1][2]);
  var_dump($a);
}

main();

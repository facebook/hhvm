<?hh

function foo(inout mixed $thing) {
  $thing = 5;
}

function main() {
  $a = 2;
  foo(&$a);
  var_dump($a);
}

main();

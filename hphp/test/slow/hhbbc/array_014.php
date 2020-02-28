<?hh

function heh() { return 4; }
function bar() { return darray['foo' => heh()]; }
function foo() {
  $x = bar();
  $x['foo'] = 2;
  return $x;
}
function main() {
  $x = foo();
  echo $x['foo'] . "\n";
}

<<__EntryPoint>>
function main_array_014() {
main();
}

<?hh

function four() { return 4; }
function heh() { return darray['foo' => four()]; }
function bar() { return darray['other' => heh()]; }
function foo() {
  $x = bar();
  $x['other']['foo'] = 2;
  return $x;
}
function main() {
  $x = foo();
  echo $x['other']['foo'] . "\n";
}

<<__EntryPoint>>
function main_array_016() {
main();
}

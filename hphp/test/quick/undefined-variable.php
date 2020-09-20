<?hh
function foo() {
  if (\HH\global_isset('b')) $b = 0;
  return $b;
}

function baz($x) {}
function bar() {
  if (\HH\global_isset('a')) $a = 0;
  baz($a);
}

<<__EntryPoint>>
function main() {
  foo();
  bar();
}

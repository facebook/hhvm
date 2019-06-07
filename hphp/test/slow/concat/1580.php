<?hh

function foo() {
 return "hello" . "\0" . "world
";
 }
function bar() {
  $s = foo();
  echo $s;
}

<<__EntryPoint>>
function main_1580() {
bar();
}

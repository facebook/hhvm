<?hh

function foo() {
  echo " FOO ";
  return " foo ";
}
function bar() {
  echo " hello " . foo() . "
";
  echo " hello " , foo() , "
";
}

<<__EntryPoint>>
function main_1588() {
bar();
}

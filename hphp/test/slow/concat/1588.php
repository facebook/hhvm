<?hh

function foo() :mixed{
  echo " FOO ";
  return " foo ";
}
function bar() :mixed{
  echo " hello " . foo() . "
";
  echo " hello " , foo() , "
";
}

<<__EntryPoint>>
function main_1588() :mixed{
bar();
}

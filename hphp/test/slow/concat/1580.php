<?hh

function foo() :mixed{
 return "hello" . "\0" . "world
";
 }
function bar() :mixed{
  $s = foo();
  echo $s;
}

<<__EntryPoint>>
function main_1580() :mixed{
bar();
}

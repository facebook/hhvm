<?hh

function bar() :mixed{
}
function foo() :mixed{
  $foo = bar();
  unset($foo);
}

<<__EntryPoint>>
function main_1744() :mixed{
foo();
}

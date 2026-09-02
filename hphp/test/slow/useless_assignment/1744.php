<?hh

function bar() :mixed{
}
function foo() :mixed{
  $foo = bar();
  $foo = null;
}

<<__EntryPoint>>
function main_1744() :mixed{
foo();
}

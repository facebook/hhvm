<?hh

const FOO = 25;
const BAR = 42;

function foo() :mixed{
  return BAR % FOO;
}


<<__EntryPoint>>
function main_legacyjit() :mixed{
var_dump(foo());
}

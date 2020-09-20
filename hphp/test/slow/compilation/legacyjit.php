<?hh

const FOO = 25;
const BAR = 42;

function foo() {
  return BAR % FOO;
}


<<__EntryPoint>>
function main_legacyjit() {
var_dump(foo());
}

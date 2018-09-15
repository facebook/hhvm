<?hh

function foo() {
  return Set { (string)'latest' };
}


<<__EntryPoint>>
function main_scalar() {
var_dump(foo());
}

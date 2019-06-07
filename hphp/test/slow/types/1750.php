<?hh

function foo($a) {
  return (int)$a;
}
function test() {
  var_dump(foo(false));
}

<<__EntryPoint>>
function main_1750() {
test();
}

<?hh

function foo($a) :mixed{
  return (int)$a;
}
function test() :mixed{
  var_dump(foo(false));
}

<<__EntryPoint>>
function main_1750() :mixed{
test();
}

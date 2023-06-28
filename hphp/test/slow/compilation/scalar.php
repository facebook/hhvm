<?hh

function foo() :mixed{
  return Set { (string)'latest' };
}


<<__EntryPoint>>
function main_scalar() :mixed{
var_dump(foo());
}

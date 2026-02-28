<?hh

function foo() :mixed{
 return false;
 }

<<__EntryPoint>>
function main_1561() :mixed{
foo() || exit("foobar");
}

<?hh

function foo() :mixed{
 return false;
 }

<<__EntryPoint>>
function main_1560() :mixed{
foo() || die("foobar");
}

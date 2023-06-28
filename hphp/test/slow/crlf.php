<?hh
//This test uses CRLF as line endings
/**			   
foo
bar
*/
function foo() :mixed{}

<<__EntryPoint>>
function main_crlf() :mixed{
var_dump((new ReflectionFunction('foo'))->getDocComment());
}

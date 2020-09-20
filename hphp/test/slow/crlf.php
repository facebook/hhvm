<?hh
//This test uses CRLF as line endings
/**			   
foo
bar
*/
function foo() {}

<<__EntryPoint>>
function main_crlf() {
var_dump((new ReflectionFunction('foo'))->getDocComment());
}

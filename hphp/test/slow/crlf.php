<?php
//This test uses CRLF as line endings
/**			   
foo
bar
*/
function foo() {}
var_dump((new ReflectionFunction('foo'))->getDocComment());

<?php
namespace Foo;
function f($a=array(Foo::bar=>0)) {
	reset($a);
	return key($a);
}
echo f()."\n";
?>
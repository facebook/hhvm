<?php
namespace Foo;
function f($a=array(namespace\bar=>0)) {
	reset($a);
	return key($a);
}
echo f()."\n";
?>
<?php
namespace Foo;
function f($a=namespace\bar) {
	return $a;
}
echo f()."\n";
?>
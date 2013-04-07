<?php
namespace Foo;
function f($a=Foo::bar) {
	return $a;
}
echo f()."\n";
?>
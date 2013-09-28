<?php
namespace Foo;
function f($a=array(Foo::bar)) {
	return $a[0];
}
echo f()."\n";
?>
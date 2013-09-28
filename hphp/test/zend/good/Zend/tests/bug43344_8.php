<?php
namespace Foo;
function f($a=array(namespace\bar)) {
	return $a[0];
}
echo f()."\n";
?>
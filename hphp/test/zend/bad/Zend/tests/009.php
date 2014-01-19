<?php

class foo {
	function bar () {
		var_dump(get_class());
	}
}

class foo2 extends foo {
}

foo::bar();
foo2::bar();

$f1 = new foo;
$f2 = new foo2;

$f1->bar();
$f2->bar();

var_dump(get_class());
var_dump(get_class("qwerty"));

var_dump(get_class($f1));
var_dump(get_class($f2));

echo "Done\n";
?>
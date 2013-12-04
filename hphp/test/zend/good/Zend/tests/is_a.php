<?php
function __autoload($name) {
	echo("AUTOLOAD '$name'\n");
	eval("class $name {}");
}

class BASE {
}

interface INT {
}

class A extends BASE implements INT {
}

$a = new A;
var_dump(is_a($a, "B1"));
var_dump(is_a($a, "A"));
var_dump(is_a($a, "BASE"));
var_dump(is_a($a, "INT"));
var_dump(is_subclass_of($a, "B2"));
var_dump(is_subclass_of($a, "A"));
var_dump(is_subclass_of($a, "BASE"));
var_dump(is_subclass_of($a, "INT"));

var_dump(is_subclass_of("X1", "X2"));
?>
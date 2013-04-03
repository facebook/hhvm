<?php

var_dump(func_get_arg(1,2,3));
var_dump(func_get_arg(1));
var_dump(func_get_arg());

function bar() {
	var_dump(func_get_arg(1));
}

function foo() {
	bar(func_get_arg(1));
}

foo(1,2);

echo "Done\n";
?>
<?php

function foo($a)
{
	var_dump(func_get_arg(2));	
}
foo(2, 3);
echo "\n";

?>
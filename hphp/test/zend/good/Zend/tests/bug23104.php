<?php
function foo($bar = array("a", "b", "c"))
{
	var_dump(current($bar));
}
foo();
?>
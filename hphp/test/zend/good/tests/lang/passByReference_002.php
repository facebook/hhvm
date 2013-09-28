<?php

function f(&$arg1)
{
	var_dump($arg1++);
}

f(2);

?>
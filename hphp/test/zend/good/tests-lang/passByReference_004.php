<?php

function foo(&$ref)
{
	var_dump($ref);
}

function bar($value)
{
	return $value;
}

foo(bar(5));

?>
<?php

function foo ($a)
{
	$a=sprintf("%02d",$a);
	var_dump($a);
}

$x="02";
var_dump($x);
foo($x);
var_dump($x);

?>
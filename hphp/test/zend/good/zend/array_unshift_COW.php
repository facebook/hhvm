<?php
	$a=array();
	$b=1;
	$c=&$b;
	array_unshift ($a,$b);
	$b=2;
	var_dump ($a);
?>
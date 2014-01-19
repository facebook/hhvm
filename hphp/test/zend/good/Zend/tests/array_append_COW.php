<?php
	$a=array();
	$b=1;
	$c=&$b;
	$a[]=$b;
	$b=2;
	var_dump ($a);
?>
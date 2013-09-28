<?php

/* 
 * proto bool ob_start([ string|array user_function [, int chunk_size [, bool erase]]])
 * Function is implemented in main/output.c
*/ 

function f($str) {
	ob_start();
	echo "hello";
	ob_end_flush();
	return $str;
}


var_dump(ob_start('f'));
echo "done";
?>
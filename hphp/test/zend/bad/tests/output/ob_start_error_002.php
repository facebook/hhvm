<?php
/* 
 * proto bool ob_start([ string|array user_function [, int chunk_size [, bool erase]]])
 * Function is implemented in main/output.c
*/ 

Class C {
	static function f($str) {
		return $str;
	}
}

var_dump(ob_start(array("nonExistent","f")));
var_dump(ob_start(array("C","nonExistent")));
var_dump(ob_start("C::no"));
var_dump(ob_start("no"));
echo "done"
?>
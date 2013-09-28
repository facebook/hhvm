<?php
/* 
 * proto bool ob_start([ string|array user_function [, int chunk_size [, bool erase]]])
 * Function is implemented in main/output.c
*/ 

Class C {
}

$c = new C;
var_dump(ob_start(array($c)));
echo "done"
?>
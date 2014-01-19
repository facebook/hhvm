<?php
/* 
 * proto bool ob_end_clean(void)
 * Function is implemented in main/output.c
*/ 

var_dump(ob_end_clean());

ob_start();
var_dump(ob_end_clean());

ob_start();
echo "Hello";
var_dump(ob_end_clean());

var_dump(ob_end_clean());

?>
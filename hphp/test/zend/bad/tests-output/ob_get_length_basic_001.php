<?php
/* 
 * proto int ob_get_length(void)
 * Function is implemented in main/output.c
*/ 

echo "No output buffers\n";
var_dump(ob_get_length());

ob_start();
var_dump(ob_get_length());
echo "hello\n";
var_dump(ob_get_length());
ob_flush();
$value = ob_get_length();
echo "hello\n";
ob_clean();
var_dump(ob_get_length());
var_dump($value);
ob_end_flush();

echo "No output buffers\n";
var_dump(ob_get_length());
?>
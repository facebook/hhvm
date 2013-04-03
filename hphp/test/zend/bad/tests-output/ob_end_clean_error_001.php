<?php
/* 
 * proto bool ob_end_clean(void)
 * Function is implemented in main/output.c
*/ 

$extra_arg = 1;

echo "\nToo many arguments\n";
var_dump(ob_end_clean($extra_arg));


?>
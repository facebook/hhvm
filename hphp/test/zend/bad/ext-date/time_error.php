<?php
/* 
 * proto int time(void)
 * Function is implemented in ext/date/php_date.c
*/ 

// Extra arguments are ignored
$extra_arg = 1;
echo "\nToo many arguments\n";
var_dump (time($extra_arg));
?>
===DONE===
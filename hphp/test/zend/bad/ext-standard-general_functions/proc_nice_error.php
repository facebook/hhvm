<?php
echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$priority = 1;

$extra_arg = 1;

var_dump(proc_nice( $priority, $extra_arg) );

var_dump(proc_nice(  ) );


?>
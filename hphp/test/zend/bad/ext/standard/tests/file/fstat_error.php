<?php
$fp = fopen (__FILE__, 'r');
$extra_arg = 'nothing'; 

var_dump(fstat( $fp, $extra_arg ) );
var_dump(fstat());

fclose($fp);

?>
===DONE===
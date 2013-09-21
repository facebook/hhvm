<?php
$fp = fopen (__FILE__, 'r');
var_dump(fstat( $fp ) );
fclose($fp);
?>
===DONE===
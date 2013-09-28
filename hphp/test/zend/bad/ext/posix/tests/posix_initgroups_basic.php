<?php
echo "Basic test of POSIX posix_initgroups function\n"; 
var_dump(posix_initgroups('foo', 'bar'));
var_dump(posix_initgroups(NULL, NULL));

?>
===DONE====
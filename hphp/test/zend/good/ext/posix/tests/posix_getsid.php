<?php
echo "*** Testing posix_getsid() : function test ***\n";

$pid = posix_getpid();
echo "\n-- Testing posix_getsid() function with current process pid --\n";
var_dump( is_long(posix_getsid($pid)) );

?>
===DONE===
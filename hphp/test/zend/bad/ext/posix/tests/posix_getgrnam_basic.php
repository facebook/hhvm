<?php
echo "Basic test of POSIX posix_getgrnam function\n"; 		

var_dump(posix_getgrnam(NULL));
var_dump(posix_getgrnam(1));
var_dump(posix_getgrnam(''));

?>
===DONE===
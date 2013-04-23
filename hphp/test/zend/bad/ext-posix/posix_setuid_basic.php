<?php

$myuid = posix_getuid();
$uid = var_dump(posix_setuid( $myuid ) );

?>
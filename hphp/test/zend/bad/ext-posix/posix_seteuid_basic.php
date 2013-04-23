<?php

$myuid = posix_geteuid();
$uid = var_dump(posix_seteuid( $myuid ) );

?>
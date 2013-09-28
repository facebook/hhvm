<?php
$gid = PHP_INT_MAX; // obscene high gid
var_dump(posix_getgrgid($gid));
var_dump(posix_getgrgid(-1));
var_dump(posix_getgrgid());
?>
===DONE===
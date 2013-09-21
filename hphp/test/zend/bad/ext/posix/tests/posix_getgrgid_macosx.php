<?php
$grp = posix_getgrgid(-1);
var_dump($grp['name']);
?>
===DONE===
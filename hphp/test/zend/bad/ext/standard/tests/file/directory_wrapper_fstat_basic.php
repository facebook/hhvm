<?php
$d = dirname(__FILE__);
$h = opendir($d);
var_dump(fstat($h));
closedir($h);
?>
===DONE===
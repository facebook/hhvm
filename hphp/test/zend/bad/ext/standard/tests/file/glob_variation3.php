<?php
$path = dirname(__FILE__);

ini_set('open_basedir', NULL);
var_dump(glob("$path/*.none"));

ini_set('open_basedir', $path);
var_dump(glob("$path/*.none"));

?>
==DONE==
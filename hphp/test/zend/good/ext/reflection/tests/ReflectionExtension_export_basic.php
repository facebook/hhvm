<?php
ob_start();
ReflectionExtension::export("reflection", true);
$test = ob_get_clean();
var_dump(empty($test));
unset($test);
ob_start();
ReflectionExtension::export("reflection", false);
$test = ob_get_clean();
var_dump(empty($test));
?>
==DONE==

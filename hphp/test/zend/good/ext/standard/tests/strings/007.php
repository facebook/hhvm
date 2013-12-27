<?php
$file = str_repeat("A", PHP_MAXPATHLEN - strlen(__DIR__ . DIRECTORY_SEPARATOR . __FILE__));

var_dump(php_strip_whitespace($file));
var_dump(ob_get_contents());

?>
===DONE===
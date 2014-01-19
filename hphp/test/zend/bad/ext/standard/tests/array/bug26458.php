<?php
$test = array("A\x00B" => "Hello world");
var_dump($test);
var_export($test);
debug_zval_dump($test);
?>
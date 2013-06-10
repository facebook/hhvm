<?php
$foo = filter_input(INPUT_GET, 'foo', FILTER_VALIDATE_INT, array('flags' => FILTER_REQUIRE_SCALAR, 'options' => array('default' => 23)));
var_dump($foo);
?>
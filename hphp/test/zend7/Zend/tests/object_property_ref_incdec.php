<?php

$obj = new stdClass;
$obj->cursor = 0;
$ref =& $obj->cursor;

$obj->cursor++;
var_dump($obj->cursor);

$obj->cursor--;
var_dump($obj->cursor);

?>

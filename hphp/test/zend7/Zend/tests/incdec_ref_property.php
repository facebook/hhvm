<?php

$obj = new stdClass;
$obj->prop = 1;
$ref =& $obj->prop;
var_dump(++$obj->prop);
var_dump($obj->prop);
var_dump($obj->prop++);
var_dump($obj->prop);
var_dump(--$obj->prop);
var_dump($obj->prop);
var_dump($obj->prop--);
var_dump($obj->prop);

?>

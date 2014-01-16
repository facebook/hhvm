<?php
$obj = new stdClass;
list($a,$b) = $obj->prop = [1,2];
var_dump($a,$b);
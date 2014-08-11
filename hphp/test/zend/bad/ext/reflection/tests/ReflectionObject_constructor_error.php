<?php

var_dump(new ReflectionObject());
var_dump(new ReflectionObject('stdClass'));
$myInstance = new stdClass;
var_dump(new ReflectionObject($myInstance, $myInstance));
var_dump(new ReflectionObject(0));
var_dump(new ReflectionObject(null));
var_dump(new ReflectionObject(array(1,2)));
?>

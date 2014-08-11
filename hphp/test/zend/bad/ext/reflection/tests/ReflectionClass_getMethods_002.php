<?php
$rc = new ReflectionClass("ReflectionClass");
echo "\nTest invalid arguments:";
$rc->getMethods('X');
$rc->getMethods('X', true);

?>

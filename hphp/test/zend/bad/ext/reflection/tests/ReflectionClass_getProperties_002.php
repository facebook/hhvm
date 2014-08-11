<?php
$rc = new ReflectionClass("ReflectionClass");
echo "\nTest invalid arguments:";
$rc->getProperties('X');
$rc->getProperties('X', true);
?>

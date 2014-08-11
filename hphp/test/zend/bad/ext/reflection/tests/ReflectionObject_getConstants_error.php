<?php
class X {
}

$rc = new ReflectionObject(new X);

$rc->getConstants('X');
$rc->getConstants(true);

?>

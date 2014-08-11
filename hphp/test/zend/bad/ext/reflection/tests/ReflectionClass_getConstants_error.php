<?php
class X {
}

$rc = new reflectionClass('X');

//Test invalid arguments
$rc->getConstants('X');
$rc->getConstants(true);
$rc->getConstants(null);
$rc->getConstants('A', 'B');

?>

<?php
interface I1 {}
interface I2 {}
interface I3 {}
interface I4 extends I3 {}
interface I5 extends I4 {}
interface I6 extends I5, I1, I2 {}
interface I7 extends I6 {}

$rc = new ReflectionClass('I7');
$interfaces = $rc->getInterfaces();
print_r($interfaces);
?>

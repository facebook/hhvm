<?php
function ReflectionParameterTest($test, $test2 = null, ...$test3) {
	echo $test;
}
$reflect = new ReflectionFunction('ReflectionParameterTest');
$params = $reflect->getParameters();
foreach($params as $key => $value) {
	echo $value->__toString() . "\n";
}
?>
==DONE==

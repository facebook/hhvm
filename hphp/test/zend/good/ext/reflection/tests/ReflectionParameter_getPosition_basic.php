<?php
function ReflectionParameterTest($test, $test2 = null) {
	echo $test;
}
$reflect = new ReflectionFunction('ReflectionParameterTest');
$params = $reflect->getParameters();
foreach($params as $key => $value) {
	var_dump($value->getPosition());
}
?>
==DONE==

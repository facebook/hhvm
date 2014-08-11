<?php
function ReflectionParameterTest($test, $test2 = null) {
	echo $test;
}
$reflect = new ReflectionFunction('ReflectionParameterTest');
$params = $reflect->getParameters();
try {
	foreach($params as $key => $value) {
		ReflectionParameter::export($reflect, $key);
	}
}
catch (ReflectionException $e) {
	echo $e->getMessage() . "\n";
}
try {
	foreach($params as $key => $value) {
		ReflectionParameter::export(42, $key);
	}
}
catch (ReflectionException $e) {
	echo $e->getMessage() . "\n";
}
?>

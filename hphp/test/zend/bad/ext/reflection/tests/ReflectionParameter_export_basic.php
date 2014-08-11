<?php
function ReflectionParameterTest($test, $test2 = null) {
	echo $test;
}
$reflect = new ReflectionFunction('ReflectionParameterTest');
foreach($reflect->getParameters() as $key => $value) {
	echo ReflectionParameter::export('ReflectionParameterTest', $key);
}
?>
==DONE==

<?php
function test() {
	echo "Hello World\n";
}

function another_test($parameter) {
	var_dump($parameter);
}

$func = new ReflectionFunction('test');
$func->invoke();

$func = new ReflectionFunction('another_test');
$func->invoke('testing');
?>

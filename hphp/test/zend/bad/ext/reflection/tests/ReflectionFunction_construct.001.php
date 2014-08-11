<?php

$a = new ReflectionFunction(array(1, 2, 3));
try {
	$a = new ReflectionFunction('nonExistentFunction');
} catch (Exception $e) {
	echo $e->getMessage();
}
$a = new ReflectionFunction();
$a = new ReflectionFunction(1, 2);
?>

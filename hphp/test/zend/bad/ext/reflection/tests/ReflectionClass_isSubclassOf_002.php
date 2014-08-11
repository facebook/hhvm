<?php
class A {}
$rc = new ReflectionClass('A');

echo "\n\nTest bad arguments:\n";
try {
	var_dump($rc->isSubclassOf());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->isSubclassOf('C', 'C'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->isSubclassOf(null));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->isSubclassOf('ThisClassDoesNotExist'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->isSubclassOf(2));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>

<?php
class C {}
$ro = new ReflectionObject(new C);

echo "\n\nTest bad arguments:\n";
try {
	var_dump($ro->isSubclassOf());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($ro->isSubclassOf('C', 'C'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($ro->isSubclassOf(null));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($ro->isSubclassOf('ThisClassDoesNotExist'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($ro->isSubclassOf(2));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>

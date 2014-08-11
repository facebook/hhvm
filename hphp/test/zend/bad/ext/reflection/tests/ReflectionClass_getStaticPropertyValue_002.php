<?php
class C {
	public static $x;
}

$rc = new ReflectionClass('C');
try {
	var_dump($rc->getStaticPropertyValue("x", "default value", 'blah'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->getStaticPropertyValue());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->getStaticPropertyValue(null));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->getStaticPropertyValue(1.5, 'def'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->getStaticPropertyValue(array(1,2,3)));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}


?>

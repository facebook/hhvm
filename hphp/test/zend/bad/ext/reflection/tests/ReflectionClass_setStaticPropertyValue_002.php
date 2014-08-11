<?php
class C {
	public static $x;
}

$rc = new ReflectionClass('C');
try {
	var_dump($rc->setStaticPropertyValue("x", "default value", 'blah'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->setStaticPropertyValue());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->setStaticPropertyValue(null));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->setStaticPropertyValue(null,null));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->setStaticPropertyValue(1.5, 'def'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	var_dump($rc->setStaticPropertyValue(array(1,2,3), 'blah'));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}


?>

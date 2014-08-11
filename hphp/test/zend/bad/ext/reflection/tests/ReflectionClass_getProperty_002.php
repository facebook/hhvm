<?php
class C {
	public $a;
}

$rc = new ReflectionClass("C");
echo "Check invalid params:\n";
try {
	var_dump($rc->getProperty());
} catch (exception $e) {
	echo $e->getMessage() . "\n";	
}
try {
	var_dump($rc->getProperty("a", "a"));
} catch (exception $e) {
	echo $e->getMessage() . "\n";	
}
try {
	var_dump($rc->getProperty(null));
} catch (exception $e) {
	echo $e->getMessage() . "\n";	
}
try {
	var_dump($rc->getProperty(1));
} catch (exception $e) {
	echo $e->getMessage() . "\n";	
}
try {
	var_dump($rc->getProperty(1.5));
} catch (exception $e) {
	echo $e->getMessage() . "\n";	
}
try {
	var_dump($rc->getProperty(true));
} catch (exception $e) {
	echo $e->getMessage() . "\n";	
}
try {
	var_dump($rc->getProperty(array(1,2,3)));
} catch (exception $e) {
	echo $e->getMessage() . "\n";	
}
try {
	var_dump($rc->getProperty(new C));
} catch (exception $e) {
	echo $e->getMessage() . "\n";	
}
?>

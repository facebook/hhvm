<?php
try {
	var_dump(new ReflectionClass());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";  
}

try {
	var_dump(new ReflectionClass(null));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";  
}

try {
	var_dump(new ReflectionClass(true));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";  
}

try {
	var_dump(new ReflectionClass(1));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";  
}

try {
	var_dump(new ReflectionClass(array(1,2,3)));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";  
}

try {
	var_dump(new ReflectionClass("stdClass", 1));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";  
}

try {
	var_dump(new ReflectionClass("X"));
} catch (Exception $e) {
	echo $e->getMessage() . "\n";  
}

?>

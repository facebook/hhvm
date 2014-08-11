<?php

class TestClass
{
    public function foo() {
    }
}


try {
	echo "Too few arguments:\n";
	$methodInfo = new ReflectionMethod();
} catch (Exception $e) {
	print $e->__toString();
}
try {
	echo "\nToo many arguments:\n";
	$methodInfo = new ReflectionMethod("TestClass", "foo", true);
} catch (Exception $e) {
	print $e->__toString();
}

?>

<?php

class TestClass
{
    public function foo() {
    }
}


try {
	echo "\nWrong type of argument (bool):\n";
	$methodInfo = new ReflectionMethod(true);
} catch (Exception $e) {
	print $e->__toString();
}
try {
	echo "\nWrong type of argument (int):\n";
	$methodInfo = new ReflectionMethod(3);
} catch (Exception $e) {
	print $e->__toString();
}
try {
	echo "\nWrong type of argument (bool, string):\n";
	$methodInfo = new ReflectionMethod(true, "foo");
} catch (Exception $e) {
	print $e->__toString();
}
try {
	echo "\nWrong type of argument (string, bool):\n";
	$methodInfo = new ReflectionMethod('TestClass', true);
} catch (Exception $e) {
	print $e->__toString();
}
try {
	echo "\nNo method given:\n";
	$methodInfo = new ReflectionMethod("TestClass");
} catch (Exception $e) {
	print $e->__toString();
}
try {
	echo "\nClass and Method in same string, bad method name:\n";
	$methodInfo = new ReflectionMethod("TestClass::foop::dedoop");
} catch (Exception $e) {
	print $e->__toString();
}
try {
	echo "\nClass and Method in same string, bad class name:\n";
	$methodInfo = new ReflectionMethod("TestCla::foo");
} catch (Exception $e) {
	print $e->__toString();
}
try {
	echo "\nClass and Method in same string (ok):\n";
	$methodInfo = new ReflectionMethod("TestClass::foo");
} catch (Exception $e) {
	print $e->__toString();
}

?>

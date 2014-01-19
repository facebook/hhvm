<?php
class C {
	static function returnConstantByValue() {
		return 100;
	}
	
	static function &returnConstantByRef() {
		return 100;
	}
	
	static function &returnVariableByRef() {
		return $GLOBALS['a'];
	}
	
	static function &returnFunctionCallByRef($functionToCall) {
		return C::$functionToCall();
	}
}

echo "\n---> 1. Via a return by ref function call, assign by reference the return value of a function that returns by value:\n";
unset($a, $b);
$a = 4;
$b = &C::returnFunctionCallByRef('returnConstantByValue');
$a++;
var_dump($a, $b);

echo "\n---> 2. Via a return by ref function call, assign by reference the return value of a function that returns a constant by ref:\n";
unset($a, $b);
$a = 4;
$b = &C::returnFunctionCallByRef('returnConstantByRef');
$a++;
var_dump($a, $b);

echo "\n---> 3. Via a return by ref function call, assign by reference the return value of a function that returns by ref:\n";
unset($a, $b);
$a = 4;
$b = &C::returnFunctionCallByRef('returnVariableByRef');
$a++;
var_dump($a, $b);

?>
<?php
class C {
	function returnConstantByValue() {
		return 100;
	}
	
	function &returnConstantByRef() {
		return 100;
	}
	
	function &returnVariableByRef() {
		return $GLOBALS['a'];
	}
	
	function &returnFunctionCallByRef($functionToCall) {
		return $this->$functionToCall();
	}
}
$c = new C;

echo "\n---> 1. Via a return by ref function call, assign by reference the return value of a function that returns by value:\n";
unset($a, $b);
$a = 4;
$b = &$c->returnFunctionCallByRef('returnConstantByValue');
$a++;
var_dump($a, $b);

echo "\n---> 2. Via a return by ref function call, assign by reference the return value of a function that returns a constant by ref:\n";
unset($a, $b);
$a = 4;
$b = &$c->returnFunctionCallByRef('returnConstantByRef');
$a++;
var_dump($a, $b);

echo "\n---> 3. Via a return by ref function call, assign by reference the return value of a function that returns by ref:\n";
unset($a, $b);
$a = 4;
$b = &$c->returnFunctionCallByRef('returnVariableByRef');
$a++;
var_dump($a, $b);

?>
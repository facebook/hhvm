<?php
function brokenTrace($arg1, &$arg2, $arg3){
	backtraceWrapper();
	var_dump(func_get_args());
	unset($arg3);
	var_dump(func_get_arg(0));
	var_dump(func_get_arg(1));
	var_dump(func_get_arg(2));
	var_dump(func_get_arg(3));
	backtraceWrapper();
	unset($arg1);
	var_dump(func_get_args());
	backtraceWrapper();
	unset($arg2);
	backtraceWrapper();
	var_dump(func_get_arg(0));
	var_dump(func_get_arg(1));
	var_dump(func_get_arg(2));
	var_dump(func_get_arg(3));
}
$arg2 = "2nd";
brokenTrace("1st", $arg2, "3th", "4th");
function backtraceWrapper(){
	$bt = debug_backtrace();
	var_dump($bt[1]['args']);
}
?>

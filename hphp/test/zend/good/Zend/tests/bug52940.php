<?php
function foo($a) {
	$a++;
	var_dump($a);	
}
function bar(&$a) {
        $a++;
        var_dump($a);
}
$a = 1;
call_user_func_array("foo", array(&$a));
var_dump($a);
call_user_func_array("bar", array(&$a));
var_dump($a);
?>
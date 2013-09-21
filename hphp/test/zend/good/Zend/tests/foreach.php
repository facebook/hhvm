<?php
$foo = array(1,2,3,4);
foreach($foo as $key => &$val) {
	if($val == 3) {
		$foo[$key] = 0;
	} else {
		$val++;
	}
}
var_dump($foo);
?>
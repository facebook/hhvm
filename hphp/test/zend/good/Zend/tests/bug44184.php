<?php
function foo() {
	$x = array(1,2,3);
	foreach ($x as $a) {
		while (1) {
			throw new Exception();
		}
	    return;
	}
}
try {
	foo();
} catch (Exception $ex) {
	echo "ok\n";
}
?>
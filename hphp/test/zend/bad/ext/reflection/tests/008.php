<?php

$a = array("", 1, "::", "a::", "::b", "a::b");

foreach ($a as $val) {
	try {
		new ReflectionMethod($val);
	} catch (Exception $e) {
		var_dump($e->getMessage());
	}
}
 
$a = array("", 1, "");
$b = array("", "", 1);
 
foreach ($a as $key=>$val) {
	try {
		new ReflectionMethod($val, $b[$key]);
	} catch (Exception $e) {
		var_dump($e->getMessage());
	}
}

echo "Done\n";
?>

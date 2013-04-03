<?php

$arr = array("a" => 1, "b" => 2);
foreach ($arr as $key => $val) {
	unset($GLOBALS[$key]);
}

var_dump($arr);
echo "Done\n";
?>
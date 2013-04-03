<?php
echo "\nNot an array.\n";
$a = TRUE;
foreach ($a as $v) {
	var_dump($v);
}

$a = null;
foreach ($a as $v) {
	var_dump($v);
}

$a = 1;
foreach ($a as $v) {
	var_dump($v);
}

$a = 1.5;
foreach ($a as $v) {
	var_dump($v);
}

$a = "hello";
foreach ($a as $v) {
	var_dump($v);
}

echo "done.\n";
?>
<?php

$a = new SplFixedArray('');

try {
	var_dump($a[1]);
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}
try {
	$a[1] = 1;
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}
try {
	var_dump(count($a[1]));
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}
try {
	var_dump($a->getSize());
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}
try {
	foreach ($a as $v) {
	}
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}
try {
	var_dump($a->setSize(10));
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

echo "Done\n";
?>

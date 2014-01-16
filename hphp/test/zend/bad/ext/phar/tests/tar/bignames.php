<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.tar';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.3.tar';
$fname4 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.4.tar';
$pname = 'phar://' . $fname;

$p1 = new PharData($fname);
$p1[str_repeat('a', 100) . '/b'] = 'hi';
$p1[str_repeat('a', 155) . '/' . str_repeat('b', 100)] = 'hi2';
copy($fname, $fname2);
$p2 = new PharData($fname2);
echo $p2[str_repeat('a', 100) . '/b']->getContent() . "\n";
echo $p2[str_repeat('a', 155) . '/' . str_repeat('b', 100)]->getContent() . "\n";

try {
	$p2[str_repeat('a', 400)] = 'yuck';
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

try {
	$p2 = new PharData($fname3);
	$p2[str_repeat('a', 101)] = 'yuck';
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

try {
	$p2 = new PharData($fname4);
	$p2[str_repeat('b', 160) . '/' . str_repeat('a', 90)] = 'yuck';
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.tar');
@unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.3.tar');
@unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.4.tar');
?>
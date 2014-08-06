<?php 
$v = 1;
for ($i = 1; $i < 10; $i++) {
	$v /= 10;
	echo "{$v} ".unserialize(serialize($v))."\n";
}
?>

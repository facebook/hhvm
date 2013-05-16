<?php // $Id$ vim600:syn=php
$v = 0.005;
for ($i = 1; $i < 10; $i++) {
	echo "round({$v}, 2) -> ".round($v, 2)."\n";
	$v += 0.01;
}
?>
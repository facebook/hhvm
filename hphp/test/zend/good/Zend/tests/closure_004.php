<?php

function run () {
	$x = 4;

	$lambda1 = function () use ($x) {
		echo "$x\n";
	};

	$lambda2 = function () use (&$x) {
		echo "$x\n";
		$x++;
	};

	return array($lambda1, $lambda2);
}

list ($lambda1, $lambda2) = run();

$lambda1();
$lambda2();
$lambda1();
$lambda2();

echo "Done\n";
?>
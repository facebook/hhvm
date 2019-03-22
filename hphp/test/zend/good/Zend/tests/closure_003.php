<?php

function run () {
	$x = 4;

	$lambda1 = function () use ($x) {
		echo "$x\n";
	};

	$lambda1();
	$x++;
	$lambda1();
}

run();

echo "Done\n";

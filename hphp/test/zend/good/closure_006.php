<?php

$getClosure = function ($v) {
	return function () use ($v) {
		echo "Hello World: $v!\n";
	};
};

$closure = $getClosure (2);
$closure ();

echo "Done\n";
?>
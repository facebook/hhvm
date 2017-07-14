<?php

function gen1() {
	try {
		yield 1;
		yield 2;
		return true;
	} finally {
		echo "Inner finally\n";
	}
}

function gen2() {
	try {
		echo "Entered try/catch\n";
		var_dump(yield from gen1());
	} finally {
		echo "Finally\n";
	}
}

$generator = gen2();

var_dump($generator->current());

unset($generator);

echo "Done\n";

?>

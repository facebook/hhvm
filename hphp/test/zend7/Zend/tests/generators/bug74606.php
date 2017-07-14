<?php

function gen() {
	$array = ["foo"];
	$array[] = "bar";

	foreach ($array as $item) {
		try {
			try {
				yield;
			} finally {
				echo "fin $item\n";
			}
		} catch (\Exception $e) {
			echo "catch\n";
			continue;
		}
	}
}
gen()->throw(new Exception);

?>

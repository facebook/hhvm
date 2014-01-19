<?php
	/* test doubles around -4e21 */
	$values = [
		-4000000000000001048576.,
		-4000000000000000524288.,
		-4000000000000000000000.,
		-3999999999999999475712.,
		-3999999999999998951424.,
	];
	/* see if we're rounding negative numbers right */
	$values[] = -2147483649.8;

	foreach ($values as $v) {
		var_dump((int)$v);
	}

?>
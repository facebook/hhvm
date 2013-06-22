<?php

function xrange($start, $end, $step = 1) {
	for ($i = $start; $i <= $end; $i += $step) {
		yield $i;
	}
}

foreach (xrange(10, 20, 2) as $i) {
	var_dump($i);
}

?>
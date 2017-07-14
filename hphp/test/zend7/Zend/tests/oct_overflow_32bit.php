<?php

$doubles = array(
	076545676543223,
	032325463734,
	0777777,
	07777777777777,
	033333333333333,
	);

foreach ($doubles as $d) {
	$l = (double)$d;
	var_dump($l);
}

echo "Done\n";
?>

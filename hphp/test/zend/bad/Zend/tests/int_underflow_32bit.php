<?php

$doubles = array(
	-2147483648,
	-2147483649,
	-2147483658,
	-2147483748,
	-2147484648,
	);

foreach ($doubles as $d) {
	$l = (int)$d;
	var_dump($l);
}

echo "Done\n";
?>
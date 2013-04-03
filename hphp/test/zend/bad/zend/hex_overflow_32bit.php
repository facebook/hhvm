<?php

$doubles = array(
	0x1736123FFFAAA,
	0XFFFFFFFFFFFFFFFFFF,
	0xAAAAAAAAAAAAAAEEEEEEEEEBBB,
	0x66666666666666666777777,
	);

foreach ($doubles as $d) {
	$l = $d;
	var_dump($l);
}

echo "Done\n";
?>
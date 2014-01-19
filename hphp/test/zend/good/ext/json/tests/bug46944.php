<?php

for ($i = 1; $i <= 16; $i++) {
	$first = 0xf0|($i >> 2);
	$second = 0x8f|($i & 3) << 4;
	$string = sprintf("aa%c%c\xbf\xbdzz", $first, $second);
	echo json_encode($string) . "\n";
}


echo "Done\n";
?>
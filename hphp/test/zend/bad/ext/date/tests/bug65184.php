<?php
	setlocale(LC_ALL, 'Japanese_Japan.932');
	/* timestamp has to be some wednesday */
	$s = strftime('%A', 1372884126);

	for ($i = 0; $i < strlen($s); $i++) {
		printf("%x ", ord($s[$i]));
	}
	echo "\n";

	echo strlen(strftime('%A')), "\n";
?>
===DONE===
<?php
	error_reporting (E_ALL ^ E_NOTICE);
	echo MCRYPT_TWOFISH."\n";
	echo MCRYPT_MODE_CBC."\n";

	const MODE1 = MCRYPT_MODE_CBC;
	echo MODE1."\n";

	const CIPHER = MCRYPT_TWOFISH;
	const MODE2 = MCRYPT_MODE_CBC;
	const MODE3 = MCRYPT_CBC;

	printf ("cipher=".CIPHER. " mode1=".MODE2. " mode2=". MODE3."\n");
?>

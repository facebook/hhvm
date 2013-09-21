<?php
	error_reporting (E_ALL ^ E_NOTICE);
	echo MCRYPT_TWOFISH."\n";
	echo MCRYPT_MODE_CBC."\n";

	define ("MODE1", MCRYPT_MODE_CBC);
	echo MODE1."\n";

	define ("CIPHER", MCRYPT_TWOFISH);
	define ("MODE2", MCRYPT_MODE_CBC);
	define ("MODE3", MCRYPT_CBC);

	printf ("cipher=".CIPHER. " mode1=".MODE2. " mode2=". MODE3."\n");
?>
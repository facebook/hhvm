<?php
foreach (mcrypt_list_algorithms() as $algo) {
	if (in_array($algo, array('rijndael-256', 'des', 'blowfish', 'twofish'))) {
	   echo "FOUND\n";
	}
}
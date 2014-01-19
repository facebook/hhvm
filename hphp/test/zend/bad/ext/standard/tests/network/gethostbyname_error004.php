<?php
	$ip = gethostbyname("www.php.net");
	var_dump((bool) ip2long($ip));
?>
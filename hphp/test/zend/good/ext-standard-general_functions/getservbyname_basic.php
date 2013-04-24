<?php

	$services = array('http', 'ftp', 'ssh', 'telnet', 'imap', 'smtp', 'nicname', 'gopher', 'finger', 'pop3', 'www');

	foreach ($services as $service) {
    		$port = getservbyname($service, 'tcp');
    		var_dump($port);
	}


?>
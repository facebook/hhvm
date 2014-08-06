<?php
	require_once("connect.inc");

	var_dump($driver->embedded);
	var_dump($driver->client_version);
	var_dump($driver->client_info);
	var_dump($driver->driver_version);
	var_dump($driver->reconnect);
	var_dump($driver->report_mode);
	print "done!";
?>

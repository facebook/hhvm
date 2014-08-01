<?php
	require_once('reflection_tools.inc');
	$class = new ReflectionClass('mysqli_driver');
	inspectClass($class);
	print "done!";
?>
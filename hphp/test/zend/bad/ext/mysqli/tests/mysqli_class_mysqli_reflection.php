<?php
	require_once('reflection_tools.inc');
	$class = new ReflectionClass('mysqli');
	inspectClass($class);
	print "done!\n";
?>
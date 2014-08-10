<?php
	require_once('reflection_tools.inc');
	$class = new ReflectionClass('mysqli_warning');
	inspectClass($class);
	print "done!\n";
?>
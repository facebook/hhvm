<?php
	require_once('reflection_tools.inc');
	$class = new ReflectionClass('mysqli_result');
	inspectClass($class);
	print "done!";
?>
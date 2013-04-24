<?php
	$f = fopen(__FILE__, 'r');
	fclose($f);
	var_dump(is_resource($f));
?>
<?php
set_error_handler('errorHandler');
try {
	if ($a) {
		echo "1\n";
	} else {
		echo "0\n";
	}
	echo "?\n";
} catch(Exception $e) { 
  echo "This Exception should be catched\n";
}
function errorHandler($errno, $errstr, $errfile, $errline, $vars) {
	throw new Exception('Some Exception');
}
?>
<?php

function foo($foo) {
	echo "Executing foo\n";
}

set_error_handler(function($errno, $errstr) {
	throw new Exception($errstr);
});

try {
	foo();
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

?>

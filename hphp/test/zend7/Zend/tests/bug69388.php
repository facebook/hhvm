<?php

error_reporting(E_ALL | E_STRICT);
function handle_error($code, $message, $file, $line, $context) {
	if (!function_exists("bla")) {
		eval('function bla($s) {echo "$s\n";}');
	}
	bla($message);
}

set_error_handler('handle_error');
eval('namespace {use Exception;}');

?>

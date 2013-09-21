<?php 

ini_set('unserialize_callback_func','check');

function check($name) {
	var_dump($name);
	throw new exception;
}

try {
	@unserialize('O:3:"FOO":0:{}');
}
catch (Exception $e) {
	/* ignore */
}

?>
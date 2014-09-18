<?php

set_error_handler(function($errno, $errmsg) {
	var_dump($errno == E_RECOVERABLE_ERROR);
}, E_RECOVERABLE_ERROR);

$closure = function() {};

var_dump($closure->prop);

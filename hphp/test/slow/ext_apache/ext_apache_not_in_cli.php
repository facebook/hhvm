<?php
// Not available in CLI mode
var_dump(function_exists('apache_get_config'));
var_dump(function_exists('apache_request_headers'));
var_dump(function_exists('getallheaders'));
var_dump(extension_loaded('apache'));

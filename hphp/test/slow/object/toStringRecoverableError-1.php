<?php
function my_handler($errno, $errmsg) { return true; }
set_error_handler('my_handler');
var_dump((string)(new stdclass));

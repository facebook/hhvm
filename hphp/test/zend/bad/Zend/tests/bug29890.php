<?php
function customErrorHandler($fErrNo,$fErrStr,$fErrFile,$fErrLine,&$fClass) {
echo "error :".$fErrStr."\n";
}

set_time_limit(5);

error_reporting(E_ALL);

set_error_handler("customErrorHandler");

define("TEST",2);

//should return a notice that the constant is already defined

define("TEST",3);

?>

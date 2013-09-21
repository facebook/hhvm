<?php
register_shutdown_function(function(){echo("\n\n!!!shutdown!!!\n\n");});
set_error_handler(function($errno, $errstr, $errfile, $errline){
 echo "error($errstr)";
 throw new Exception("Foo");
});

require 'notfound.php';
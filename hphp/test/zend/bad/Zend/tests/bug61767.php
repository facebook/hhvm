<?php
set_error_handler(function($code, $msg, $file = null, $line = null) {
    echo "Error handler called ($msg)\n";
    throw new \ErrorException($msg, $code, 0, $file, $line);
});

register_shutdown_function(function(){
    echo "Shutting down\n";
    print_r(error_get_last());
});

//$undefined = null; // defined variable does not cause problems
$undefined->foo();
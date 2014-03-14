<?php
//Tests that calling is_callable() on an illegal array just returns false, 
//without an array->string conversion notice

function myErrorHandler($errno, $errstr, $errfile, $errline)
{
    //if ($errno == E_USER_NOTICE)
        echo "Notice: $errstr\n";
}

$old_error_handler = set_error_handler("myErrorHandler");
//error_reporting(0);
var_dump( is_callable( array(1,2,3), true, $name ) );
echo $name . "\n";
//set_error_handler($old_error_handler);
?>

<?php
//Tests that calling is_callable() on an illegal array just returns false,
//without an array->string conversion notice

function myErrorHandler($errno, $errstr, $errfile, $errline)
{
        echo "Notice: $errstr\n";
}

set_error_handler("myErrorHandler");
var_dump( is_callable( array(1,2,3), true, $name ) );
echo $name . "\n";
?>

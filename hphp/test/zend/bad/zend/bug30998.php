<?php
error_reporting(-1);
                           
function my_error($errno, $errstr, $errfile, $errline)
{
        print "$errstr ($errno) in $errfile:$errline\n";
        return false;
}
set_error_handler('my_error');
                           
$f = fopen("/tmp/blah", "r");
?>
===DONE===
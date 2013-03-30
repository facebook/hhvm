<?php
function userErrorHandler($num, $msg, $file, $line, $vars)
{
    debug_print_backtrace();
}

$OldErrorHandler = set_error_handler("userErrorHandler");

function GenerateError1($A1)
{
    $a = $b;
}

function GenerateError2($A1)
{
    GenerateError1("Test1");
}

GenerateError2("Test2");
?>
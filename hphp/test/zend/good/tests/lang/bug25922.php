<?php

function my_error_handler($error, $errmsg='', $errfile='', $errline=0, $errcontext='')
{
    echo "$errmsg\n";
    $errcontext = '';
}

function test()
{
    echo "Undefined index here: '{$data['HTTP_HEADER']}'\n";
}

<<__EntryPoint>> function main() {
set_error_handler('my_error_handler');

test();
}

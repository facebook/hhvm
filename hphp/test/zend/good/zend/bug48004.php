<?php
function error_handler($errno, $errstr, $errfile, $errline, $errcontext) {
        return true;
}

function test() {
        $data->id = 1;
        print_r($data);
}

set_error_handler("error_handler");
test();
?>
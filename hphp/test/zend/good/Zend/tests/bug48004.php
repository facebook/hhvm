<?php
function error_handler($errno, $errstr, $errfile, $errline, $errcontext) {
        return true;
}

function test() {
        $data->id = 1;
        print_r($data);
}
<<__EntryPoint>> function main() {
set_error_handler("error_handler");
test();
}

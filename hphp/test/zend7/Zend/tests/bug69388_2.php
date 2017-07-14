<?php
error_reporting(E_ALL | E_STRICT);
function handle_error($code, $message, $file, $line, $context) {
    eval('namespace Foo;');
    echo "$message\n";
}

set_error_handler('handle_error');
eval('namespace {use Exception;}');

?>

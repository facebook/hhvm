<?php

function doSomething(string $value = UNDEFINED) {
}

set_error_handler(function($errno, $errstr) {
    throw new Exception($errstr);
});

doSomething();

?>

<?php

function test(array... $args) {
    var_dump($args);
}

set_error_handler(function($errno, $errstr) {
    var_dump($errstr);
    return true;
});

test([0], [1], 2);

?>

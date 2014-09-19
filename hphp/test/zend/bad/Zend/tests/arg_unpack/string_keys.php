<?php

set_error_handler(function($errno, $errstr) {
    var_dump($errstr);
});

var_dump(...[1, 2, "foo" => 3, 4]);
var_dump(...new ArrayIterator([1, 2, "foo" => 3, 4]));

?>

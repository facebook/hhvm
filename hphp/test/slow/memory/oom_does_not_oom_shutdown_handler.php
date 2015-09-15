<?php

ini_set('memory_limit', '5M');

register_shutdown_function(function () {
    print "IN SHUTDOWN".PHP_EOL;
});

function foo() {
  $x = str_repeat('x', 1024 * 1024 * 20);
}

foo();

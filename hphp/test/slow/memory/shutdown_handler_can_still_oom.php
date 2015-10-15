<?php

ini_set('memory_limit', '5M');
$rep = 'x';

register_shutdown_function(function () {
    global $rep;
    print "IN SHUTDOWN".PHP_EOL;
    $y = str_repeat($rep, 1024 * 1024 * 15);
});

function foo() {
  global $rep;
  $x = str_repeat($rep, 1024 * 1024 * 20);
}

foo();

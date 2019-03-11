<?php

ini_set('memory_limit', '18M');
MemoryOomDoesNotOomShutdownHandler::$rep = 'x';

register_shutdown_function(function () {
    print "IN SHUTDOWN".PHP_EOL;
    $y = str_repeat(MemoryOomDoesNotOomShutdownHandler::$rep, 1024 * 64);
    echo strlen($y).PHP_EOL;
});

function foo() {

  $x = str_repeat(MemoryOomDoesNotOomShutdownHandler::$rep, 1024 * 1024 * 20);
}

foo();

abstract final class MemoryOomDoesNotOomShutdownHandler {
  public static $rep;
}

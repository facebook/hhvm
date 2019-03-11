<?php

ini_set('memory_limit', '16M');
MemoryShutdownHandlerCanStillOom::$rep = 'x';

register_shutdown_function(function () {
    print "IN SHUTDOWN".PHP_EOL;
    $y = str_repeat(MemoryShutdownHandlerCanStillOom::$rep, 1024 * 1024 * 15);
});

function foo() {

  $x = str_repeat(MemoryShutdownHandlerCanStillOom::$rep, 1024 * 1024 * 20);
}

foo();

abstract final class MemoryShutdownHandlerCanStillOom {
  public static $rep;
}

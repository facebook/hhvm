<?php

$t = new IntervalTimer(1, function() { echo "ping\n"; });
$t->start();
$n = microtime(true);
while (microtime(true) < $n + 1) {}
$t->stop();
